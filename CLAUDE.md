# Claude Desktop Buddy — Project Reference

Complete hardware and firmware documentation for the XIAO ESP32-S3 + Seeed Round Display build.
This file exists so any AI assistant can pick up the project without asking for hardware clarification.

---

## Hardware

| Component | Detail |
|---|---|
| MCU | Seeed XIAO ESP32-S3 |
| Display | GC9A01 240×240 round, driven via Seeed_GFX (TFT_eSPI fork) |
| Touch IC | **CST816D** at I2C address `0x15` (NOT CHSC6X/0x2E — confirmed by schematic) |
| RTC | PCF8563 at I2C address `0x51` |
| Battery | Li-Po via JST 1.25mm on Round Display PCB, charged by XIAO TP4054 at 100mA |

### Pin Map

| Function | Arduino pin | GPIO |
|---|---|---|
| SPI SCK | D8 | GPIO7 |
| SPI MOSI | D10 | GPIO9 |
| SPI MISO | D9 | GPIO8 |
| TFT CS | D1 | GPIO2 |
| TFT DC | D3 | GPIO4 |
| TFT RST | — | -1 (not used) |
| Backlight | D6 | GPIO43 |
| I2C SDA | D4 | GPIO5 |
| I2C SCL | D5 | GPIO6 |
| Touch INT (TP_INT) | D7 | GPIO44 |
| Touch RST (TP_RST) | D2 | GPIO3 |
| Battery ADC | D0 | GPIO1 |

### 2-bit Switch on Round Display PCB
Both bits must be ON for full functionality:
- Bit 1 (ON): enables peripheral 3.3V rail → powers CST816D + PCF8563
- Bit 2 (KE): enables battery voltage divider → enables ADC battery reading

---

## Critical Hardware Discoveries

### Touch IC
- The chip is **CST816D**, not CHSC6X. Early docs/libraries say CHSC6X at 0x2E — wrong for this board revision.
- Confirmed by reading the Round Display V1.1 schematic.

### I2C Initialization Order (CRITICAL)
`tft.init()` **MUST** run before `Wire.begin(5, 6)`. If Wire is initialised first, SDA gets stuck LOW (all I2C ops timeout). `tft.init()` internally releases SDA as a side effect of the SPI peripheral setup.

The correct sequence every time:
```cpp
tft.init();
tft.setRotation(0);
// --- then I2C ---
Wire.begin(5, 6);
Wire.setClock(400000);
delay(100);
```

### CST816D Reset Sequence
After `Wire.begin()`, the CST816D requires a hardware reset to respond to I2C. The specific sequence (INT=HIGH during reset) puts the chip into I2C address mode:

```cpp
pinMode(3,  OUTPUT);   // TP_RST
pinMode(44, OUTPUT);   // TP_INT
digitalWrite(3,  LOW);
digitalWrite(44, HIGH);  // INT=HIGH during reset → I2C mode
delay(50);
digitalWrite(3,  HIGH);  // release reset
delay(100);              // CST816D firmware boot
pinMode(44, INPUT_PULLUP);
```

Without this reset, the chip is absent (err=2) after every soft reset/flash. It only responds without this sequence after a full power cycle.

### CST816D Touch Read
Standard `Wire.requestFrom(0x15, 5)` does NOT work. Must write register address first:

```cpp
Wire.beginTransmission(0x15);
Wire.write(0x01);              // start at gesture register
Wire.endTransmission(false);
Wire.requestFrom(0x15, 6);
// d[0]=gesture  d[1]=touch_count
// d[2]=XH+event  d[3]=XL  → X = ((d[2]&0x0F)<<8)|d[3]
// d[4]=YH+event  d[5]=YL  → Y = ((d[4]&0x0F)<<8)|d[5]
// touch present if d[1] > 0
```

### Battery ADC
- `analogReadMilliVolts(1)` reads through a ÷2 voltage divider
- Actual battery voltage = ADC reading × 2
- Full = 2100mV ADC (4.2V actual), Empty = 1550mV ADC (3.1V actual)
- When ADC reads < 1500mV, KE switch is OFF or no battery — hide the arc

#### Battery Percentage Function

File: `include/utils.h` (declaration) + `src/utils.cpp` (definition) — `Utils::batteryPercent()`

```cpp
// include/utils.h
namespace Utils {
  int batteryPercent();  // 0–100%, or -1 if absent / KE switch OFF
}

// src/utils.cpp
int Utils::batteryPercent()
{
  int adc = analogReadMilliVolts(1);          // GPIO1, ÷2 divider

  if (adc < 1500) return -1;                  // no battery or KE switch OFF

  // Map ADC range 1550–2100 → 0–100%
  int pct = ((adc - 1550) * 100) / (2100 - 1550);

  if (pct < 0)   pct = 0;                     // clamp floor
  if (pct > 100) pct = 100;                   // clamp ceiling (charging can push past 4.2V)

  return pct;
}
```

---

## Project Structure

```
src/
  main.cpp                 — setup, display/touch/LVGL init, loop
  utils.cpp                — Utils::batteryPercent(), Utils::initWifi()
  Screens/
    ScreenManager.cpp/h    — screen switching & lifecycle (LVGL-based)
    ClockScreen.cpp        — clock + date + battery icon
    WeatherScreen.cpp      — (stub)
    SettingsScreen.cpp     — (stub)
include/
  utils.h                  — Utils namespace declarations
  secrets.h                — WiFi credentials (git-ignored)
platformio.ini
CLAUDE.md                  — this file
```

---

## Libraries (platformio.ini lib_deps)

```
https://github.com/Seeed-Studio/Seeed_GFX.git
https://github.com/Seeed-Studio/Seeed_Arduino_RoundDisplay.git
bitbank2/AnimatedGIF @ ^2.1.1
bblanchon/ArduinoJson @ ^7.0.0
```

BLE uses the ESP32 Arduino built-in stack (BLEDevice.h, BLEServer.h) — no extra lib needed.

### platformio.ini key flags
```ini
board = seeed_xiao_esp32s3
framework = arduino
board_build.arduino.usb_cdc_on_boot = 1   ← frees GPIO43 for backlight LEDC
board_build.filesystem = littlefs
board_build.partitions = default_8MB.csv
build_flags = -DBOARD_SCREEN_COMBO=501
```

---

## Display & Rendering

- **LVGL v9** — all UI is built with LVGL widgets (labels, objects, timers)
- **GC9A01 240×240** — driven via TFT_eSPI `dispFlush` callback
- **Partial buffer** — `240 × 10` rows, `LV_DISPLAY_RENDER_MODE_PARTIAL`

### GC9A01 Vertical Lines Fix
The CHSCX6X_Init.cpp in Seeed_GFX calls `Wire.begin()` with no args inside `tft.init()`. This was patched to prevent it from overriding our explicit `Wire.begin(5,6)`:

File: `.pio/libdeps/seeed_xiao_esp32s3/Seeed_GFX/Touch_Drivers/CHSCX6X_Init.cpp`
```cpp
// Wire.begin() call removed — main.cpp owns Wire initialisation
// TOUCH_WIRE.begin();
```

---

## BLE Protocol

Nordic UART Service (NUS):
- Service: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`
- RX (desktop→device): `6e400002-...` WRITE
- TX (device→desktop): `6e400003-...` NOTIFY

All data is UTF-8 JSON, newline-delimited. Device name advertises as `Claude-XXYY`.

### Incoming from desktop
```json
{"total":3,"running":1,"waiting":1,"msg":"...","entries":[...],"tokens":184502,"tokens_today":31200}
{"prompt":{"id":"req_abc","tool":"Bash","hint":"rm -rf /tmp"}}
{"time":[1775731234,-25200]}
{"cmd":"status"}  {"cmd":"owner","name":"Felix"}  {"cmd":"unpair"}
```

### Outgoing from device
```json
{"cmd":"permission","id":"req_abc","decision":"once"}   // approve
{"cmd":"permission","id":"req_abc","decision":"deny"}   // deny
{"ack":"status","ok":true,"data":{"name":"...","sec":true,"bat":{...},"sys":{...},"stats":{...}}}
```

### Security
LE Secure Connections, DisplayOnly passkey. NUS characteristics are encrypted-only.
After auth failure, bonds are automatically cleared and reconnection starts fresh.

---

## Backlight

- LEDC channel 0, 1kHz, 8-bit, GPIO43
- `ledcSetup` + `ledcAttachPin` must run **after** `tft.init()` because `tft.init()` claims GPIO43 as plain digital output
- User-adjustable via swipe up/down: `_userBrightLevel` (default 128)
- Activity-scaled: active = `_userBrightLevel`, idle = `_userBrightLevel / 2`

---

## Touch Gesture System

Detected on finger release (`!isTouched && wasTouched`), except long press which fires mid-hold.

| Gesture | Trigger | Action |
|---|---|---|
| Swipe LEFT | `dx < 0`, `abs(dx)>35`, `abs(dx)>abs(dy)` | Previous screen (snap-black transition) |
| Swipe RIGHT | `dx > 0` | Next screen |
| Swipe UP | `dy < 0`, `abs(dy)>35`, `abs(dy)>abs(dx)` | Brightness +30 |
| Swipe DOWN | `dy > 0` | Brightness -30 |
| Long Press | held ≥800ms, movement <20px, fires mid-hold | Cycle infoPage or toggle screen |
| Double Tap | two taps within 300ms | P_CELEBRATE animation |
| Tap | default | Advance sub-page / hit-test buttons |

---

## Screen Modes

```cpp
enum Screen : uint8_t { SCREEN_CLOCK, SCREEN_WEATHER, SCREEN_SETTINGS, SCREEN_COUNT };
```

- **SCREEN_CLOCK**: time + date + battery icon (refreshed via LVGL timer)
- **SCREEN_WEATHER**: stub
- **SCREEN_SETTINGS**: stub

Screen switching via `screensSwitch(SCREEN_*)` in [ScreenManager.h](src/Screens/ScreenManager.h).

---

## Character / GIF System

Characters live in LittleFS at `/characters/<name>/`. Each character pack:
```
manifest.json   — palette, state→gif mapping, name
idle.gif        — (and other states: sleep/busy/attention/celebrate/dizzy/heart)
```

GIFs are 120×120 source, upscaled 1.5× to 180×180 for normal mode, downscaled 0.75× to 90×90 for peek (clock) mode.

Install via the Hardware Buddy window's folder drop target (BLE file transfer).
Direct USB install: `tools/flash_character.py` from the claude-desktop-buddy repo.

---

## Work Hours Schedule (currently disabled)

Deep sleep Mon-Fri 09:30–19:30 local time. Uncomment in `loop()` to re-enable:
```cpp
// ── Work-hours schedule: TEMPORARILY DISABLED ────────────────
```
TZ is set from the bridge's time sync. `_nvTzOffsetSec` persists through deep sleep via `RTC_DATA_ATTR`.

---

## Stats Persistence (NVS)

Stored under key namespace `"buddy"` via `Preferences`:
- `appr` / `deny` — saved immediately on each event
- `tok` / `lvl` — saved every 5000 tokens (prevents losing progress on reboot)
- `nap` — saved on wake from nap
- `petname` / `owner` / `species` — saved on change

---

## Known Issues / Notes

- **Vertical grey lines on display**: GC9A01 hardware column driver artifact at dark levels. Partially mitigated by adjusted VCOM/VREG values in `.pio/libdeps/.../Seeed_GFX/TFT_Drivers/GC9A01_Init.h` (C3=0x1C, C4=0x1C, C9=0x2E).
- **USB detection**: Cannot reliably detect USB vs battery-only in software. Screen sleep uses 2.5-minute no-charging grace period + `!bleConnected()` as proxy.
- **Battery charging detection**: Compares ADC readings 90 seconds apart; needs 12mV rise to declare "charging".
- **Thinking indicator**: Shows animated "thinking..." when `tama.connected && tama.nLines == 0`. Bridge can also send `{"thinking":true}` for explicit control.
