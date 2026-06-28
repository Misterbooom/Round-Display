# XIAO ESP32-S3 + Seeed Round Display — Developer Guide

Hardware reference for the GC9A01 240×240 round display, CST816D touch, PCF8563 RTC, battery ADC, and backlight. Battle-tested init sequences, not copy-paste from outdated docs.

---

## 1. Pin Map

```
         XIAO ESP32-S3
         ┌─────────────┐
    D0   │  1        14 │  D6  (BL)
    D1   │  2        13 │  D7  (TP_INT)
    D2   │  3        12 │  D8  (SCK)
    D3   │  4        11 │  D9  (MISO)
    D4   │  5        10 │  D10 (MOSI)
    D5   │  6         9 │
    3V3  │  7         8 │  GND
         └─────────────┘
```

| Function | Arduino Pin | GPIO | Notes |
|---|---|---|---|
| **SPI SCK** | D8 | 7 | Display clock |
| **SPI MOSI** | D10 | 9 | Display data |
| **SPI MISO** | D9 | 8 | Display (unused but wired) |
| **TFT CS** | D1 | 2 | Chip select |
| **TFT DC** | D3 | 4 | Data/command |
| **TFT RST** | — | -1 | Not connected |
| **Backlight** | D6 | 43 | LEDC PWM — frees UART0-TX since USB-CDC is used |
| **I2C SDA** | D4 | 5 | Shared: touch + RTC |
| **I2C SCL** | D5 | 6 | |
| **Touch INT** | D7 | 44 | Active-low interrupt from CST816D |
| **Touch RST** | D2 | 3 | CST816D hardware reset |
| **Battery ADC** | D0 | 1 | Reads through ÷2 voltage divider |

---

## 2. Critical Init Sequence

**Order matters.** Get this wrong and I2C is dead.

```
1. tft.init()         ← MUST be first — releases SDA from SPI mode
2. tft.setRotation()
3. Wire.begin(5, 6)   ← explicit pins AFTER display init
4. Wire.setClock(400000)
5. ledcSetup + ledcAttachPin for backlight  ← AFTER tft.init() (GPIO43 conflict)
6. CST816D hardware reset sequence
```

```cpp
void setup() {
  Serial.begin(115200);

  // Step 1 — Display first
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  // Step 2 — I2C immediately after
  Wire.begin(5, 6);
  Wire.setClock(400000);
  delay(100);

  // Step 3 — Backlight LEDC (AFTER tft.init — GPIO43)
  ledcSetup(0, 1000, 8);        // channel 0, 1kHz, 8-bit
  ledcAttachPin(43, 0);
  ledcWrite(0, 128);            // 50%

  // Step 4 — Reset the touch controller
  cst816dReset();
}
```

### Why `tft.init()` must come first

If `Wire.begin()` runs before `tft.init()`, **SDA gets stuck LOW** and all I2C operations timeout. `tft.init()` internally configures the SPI pins, which as a side effect releases the I2C lines. This is the single most common "my touch doesn't work" trap.

### Library patch required

The Seeed_GFX library's `CHSCX6X_Init.cpp` calls `Wire.begin()` with no arguments inside `tft.init()`. This overrides your explicit `Wire.begin(5,6)`. **Patch it:**

File: `.pio/libdeps/seeed_xiao_esp32s3/Seeed_GFX/Touch_Drivers/CHSCX6X_Init.cpp`

```cpp
// Wire.begin() call removed — main.cpp owns Wire initialisation
// TOUCH_WIRE.begin();
```

---

## 3. Touch — CST816D

### The chip is CST816D, NOT CHSC6X

Early docs and libraries claim `CHSC6X` at I2C address `0x2E`. The Round Display **V1.1** uses **CST816D** at `0x15`. Check your schematic — if your board matches, use `0x15`.

### Hardware reset sequence

After every soft reset or flash, the CST816D needs this exact sequence. Without it the chip is absent (`err=2`) on I2C scans until you do a full power cycle.

```cpp
void cst816dReset() {
  pinMode(3,  OUTPUT);    // TP_RST = GPIO3
  pinMode(44, OUTPUT);    // TP_INT = GPIO44

  digitalWrite(3,  LOW);   // assert reset
  digitalWrite(44, HIGH);  // INT=HIGH during reset → I2C address mode
  delay(50);

  digitalWrite(3,  HIGH);  // release reset
  delay(100);              // firmware boot time
  pinMode(44, INPUT_PULLUP);
}
```

The key insight: **INT must be HIGH while RST is released**, otherwise the chip boots into SPI mode instead of I2C mode.

### Reading touch data

Standard `Wire.requestFrom(0x15, 5)` does **not** work. You must write the register address first with a repeated start:

```cpp
bool readTouch(int16_t& x, int16_t& y) {
  // Write register address 0x01 (gesture register), then repeated-start read 6 bytes
  Wire.beginTransmission(0x15);
  Wire.write(0x01);
  Wire.endTransmission(false);       // false = repeated start (don't send STOP)
  Wire.requestFrom(0x15, (uint8_t)6);

  uint8_t d[6] = {0};
  for (int i = 0; i < 6 && Wire.available(); i++) d[i] = Wire.read();

  // d[0] = gesture type
  // d[1] = touch count (0 = no touch, >0 = finger down)
  // d[2] = X[11:8] + event flag
  // d[3] = X[7:0]
  // d[4] = Y[11:8] + event flag
  // d[5] = Y[7:0]

  if (d[1] == 0) return false;  // no touch

  x = ((d[2] & 0x0F) << 8) | d[3];   // mask off top nibble (event bits)
  y = ((d[4] & 0x0F) << 8) | d[5];
  return true;
}
```

### Touch detection via interrupt

The INT pin (GPIO44) goes **active-low** when a finger is present:

```cpp
pinMode(44, INPUT_PULLUP);
bool touched = (digitalRead(44) == LOW);
```

Poll this in your loop — it's fast and doesn't need I2C. Only call `readTouch()` when INT is LOW to get XY coordinates.

---

## 4. RTC — PCF8563

| Property | Value |
|---|---|
| I2C address | `0x51` |
| Shared bus | SDA=GPIO5, SCL=GPIO6 |

Standard PCF8563 library works fine. It shares the I2C bus with the touch controller — no conflicts as long as the bus init order is correct.

**Important:** The RTC is only powered when **Bit 1 of the PCB switch is ON** (enables the 3.3V peripheral rail). With the switch off, the PCF8563 disappears from the bus.

---

## 5. Battery ADC

The XIAO reads battery voltage through a **÷2 voltage divider** on the Round Display PCB. This divider is only connected when **KE switch (Bit 2) is ON**.

```cpp
// Read ADC on GPIO1 (D0)
int32_t adcMv = analogReadMilliVolts(1);   // this is HALF the actual battery voltage

// Actual battery voltage
int32_t batteryMv = adcMv * 2;

// Thresholds (calibrate to your specific battery)
// Full Li-Po  = 4.2V  → ADC reads ~2100mV
// Empty Li-Po = 3.1V  → ADC reads ~1550mV
// If ADC < 1500mV → KE switch is OFF or no battery connected
```

### Battery percentage

```cpp
#define BAT_EMPTY_MV 1550   // ~3.1V actual
#define BAT_FULL_MV  2100   // ~4.2V actual

int32_t adcMv = analogReadMilliVolts(1);
if (adcMv < 1500) {
  // No battery or KE switch off — hide battery indicator
  return;
}
int pct = (adcMv - BAT_EMPTY_MV) * 100 / (BAT_FULL_MV - BAT_EMPTY_MV);
pct = constrain(pct, 0, 100);
```

### ADC noise reduction

The ESP32 ADC is noisy. Average 40 samples for stable readings (±4mV noise floor):

```cpp
int32_t mv = 0;
for (int i = 0; i < 40; i++) mv += analogReadMilliVolts(1);
mv /= 40;
```

### Charging detection

There's no dedicated charging-status pin on the XIAO. Detect charging by watching for a **rising voltage trend** over time — a charged battery stays flat, a charging battery slowly rises:

```cpp
// Compare current reading to one 90 seconds ago
// 12mV rise over 90s = charging (above ADC noise floor)
bool charging = (mvNow - mv90SecondsAgo) >= 12;
```

---

## 6. Backlight — PWM on GPIO43

GPIO43 is normally UART0-TX, but XIAO ESP32-S3 uses **USB-CDC** for Serial, so the pin is free. Use ESP32 LEDC hardware PWM:

```cpp
// Setup — MUST run AFTER tft.init()
// tft.init() claims GPIO43 as a plain digital output
ledcSetup(0, 1000, 8);     // channel 0, 1kHz, 8-bit resolution (0-255)
ledcAttachPin(43, 0);

// Set brightness
ledcWrite(0, 128);          // 50%
ledcWrite(0, 255);          // 100%
ledcWrite(0, 30);           // minimum usable
ledcWrite(0, 0);            // off
```

### platformio.ini flag required

```ini
board_build.arduino.usb_cdc_on_boot = 1
```

Without this, GPIO43 is claimed by the hardware UART and LEDC can't use it.

---

## 7. Display — GC9A01 (240×240 round)

### Library setup (platformio.ini)

```ini
lib_deps =
  https://github.com/Seeed-Studio/Seeed_GFX.git
  https://github.com/Seeed-Studio/Seeed_Arduino_RoundDisplay.git

build_flags = -DBOARD_SCREEN_COMBO=501
```

### Sprite-based rendering

For smooth animation (~60fps), render everything into a sprite first, then push:

```cpp
TFT_eSPI    tft;
TFT_eSprite spr = TFT_eSprite(&tft);

void setup() {
  tft.init();
  tft.setRotation(0);
  spr.createSprite(240, 240);
}

void loop() {
  spr.fillSprite(TFT_BLACK);
  // ... draw into spr ...
  spr.pushSprite(0, 0);
}
```

### Vertical grey lines on dark backgrounds

The GC9A01 has a known artifact — faint vertical lines at dark grey levels. The fix is in the driver init registers (patched in the library):

File: `.pio/libdeps/.../Seeed_GFX/TFT_Drivers/GC9A01_Init.h`

```cpp
// Adjusted VCOM/VREG values to reduce column-driver artifacts
// C3=0x1C, C4=0x1C, C9=0x2E
```

This mitigates but doesn't fully eliminate the lines. They're a hardware characteristic of the GC9A01.

---

## 8. 2-bit DIP Switch on the Round Display PCB

Both switches must be **ON** for full functionality:

| Switch | Function | When OFF |
|---|---|---|
| **Bit 1** | 3.3V rail to peripherals | Touch dead, RTC dead — I2C scan shows nothing |
| **Bit 2 (KE)** | Battery voltage divider | `analogReadMilliVolts(1)` reads ~0 — no battery arc |

---

## 9. I2C Bus Scan

Quick diagnostic to see what's alive on the bus:

```cpp
void i2cScan() {
  Serial.println("I2C scan:");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  found 0x%02X\n", addr);
    }
  }
}
```

Expected output with everything working:
```
I2C scan:
  found 0x15    ← CST816D touch
  found 0x51    ← PCF8563 RTC
```

---

## 10. Quick Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| I2C scan finds nothing | Wrong init order | `tft.init()` before `Wire.begin()` |
| Touch dead after flash | CST816D needs reset after soft reset | Run the hardware reset sequence |
| Touch only works after power cycle | Missing reset sequence | Same as above |
| Battery always 0% | KE switch off or no battery | Check Bit 2 on PCB switch |
| RTC not found on I2C | Bit 1 switch off | Check Bit 1 on PCB switch |
| Backlight stuck on/off | `ledcSetup` ran before `tft.init()` | Move backlight init after display init |
| Vertical grey lines on screen | GC9A01 hardware characteristic | Check VCOM/VREG in GC9A01_Init.h |
| `Wire.requestFrom` returns 0 bytes for touch | Using wrong read method | Use the repeated-start pattern (0x15 + write register + endTransmission(false)) |
| Compile error: GPIO43 conflict | Missing USB-CDC flag | Add `usb_cdc_on_boot = 1` to platformio.ini |
