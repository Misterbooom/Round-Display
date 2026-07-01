/*
 * LVGL TEST BUTTON — XIAO ESP32-S3 + Seeed Round Display (GC9A01 240×240)
 *
 * One button, center screen. Tap it — counter goes up.
 * Every line is commented. Copy-paste ready.
 */

// ─── INCLUDES ────────────────────────────────────────────────────────────────
#include <Arduino.h>  // millis(), delay(), Serial, pinMode, digitalWrite
#include <TFT_eSPI.h> // Display driver (GC9A01 over SPI)
#include <Wire.h>     // I2C for touch controller (CHSC6X at 0x2E)
#include <lvgl.h>     // LVGL v9 graphics library
#include "Screens/ScreenManager.h"
#include <utils.h>
#include "TouchScreen.h"
#include "BLE.h"

// ╔════════════════════════════════════════════════════════════════════════════╗
// ║  DISPLAY FLUSH — LVGL calls this when it needs to send pixels to screen   ║
// ╚════════════════════════════════════════════════════════════════════════════╝

static TFT_eSPI tft = TFT_eSPI(240, 240); // display object (240×240 round)

static void dispFlush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  uint16_t w = lv_area_get_width(area);
  uint16_t h = lv_area_get_height(area);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)px_map, w * h, true);
  tft.endWrite();

  lv_display_flush_ready(disp); // tell LVGL we're done drawing
}
static uint32_t lvglTick()
{
  return static_cast<uint32_t>(millis());
}
void setup()
{

  // ── 1. Serial ──────────────────────────────────────────────────────────
  Serial.begin(115200);
  delay(3000);
  Serial.println("\n=== Round Display LVGL Test ===");

  // ── 2. Display ─────────────────────────────────────────────────────────
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  // ── 3. Backlight ───────────────────────────────────────────────────────
  pinMode(D6, OUTPUT);
  digitalWrite(D6, HIGH);

  Wire.begin();
  Wire.setClock(400000);
  delay(100);

  // Quick I2C scan — prints what's alive on the bus
  Serial.println("I2C scan:");
  for (uint8_t addr = 1; addr < 127; addr++)
  {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0)
      Serial.printf("  found 0x%02X\n", addr);
  }

  // ── 5. LVGL init ───────────────────────────────────────────────────────
  lv_init();
  lv_tick_set_cb(lvglTick);

  lv_display_t *disp = lv_display_create(240, 240);
  lv_display_set_flush_cb(disp, dispFlush);

  static lv_color_t buf[240 * 60];
  lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_make(14, 15, 14), 0);

  TouchScreen::createInstance();

  // lv_obj_t *statusLabel = lv_label_create(lv_scr_act());
  // lv_obj_align(statusLabel, LV_ALIGN_CENTER, 0, 0);
  // lv_obj_set_style_text_color(statusLabel, lv_color_white(), 0);

  // lv_label_set_text(statusLabel, "Connecting to the WiFi...");
  Utils::initRTC();
  Ble::init();
  // if (!Utils::initWifi([statusLabel](int i)
  //                      {
  //       int dots = i % 4;
  //       if (dots == 0)
  //         lv_label_set_text(statusLabel, "Connecting to the WiFi");
  //       else if (dots == 1)
  //         lv_label_set_text(statusLabel, "Connecting to the WiFi.");
  //       else if (dots == 2)
  //         lv_label_set_text(statusLabel, "Connecting to the WiFi..");
  //       else
  //         lv_label_set_text(statusLabel, "Connecting to the WiFi..."); }))
  // {
  //   lv_label_set_text(statusLabel, "Can't connect to the WiFi!");
  //   lv_timer_handler();
  //   delay(2000);
  // }
  // else
  // {
  //   lv_label_set_text(statusLabel, "Connected!");
  //   lv_timer_handler();
  //   delay(800);
  // }
  Serial.println("Initializing screens!");
  screensInit();
  TouchScreen::onSwipeLeft = nextScreen;
  TouchScreen::onSwipeRight = previousScreen;
  Ble::onWeatherReceive = WeatherScreen::updateWeather;
}

// ╔════════════════════════════════════════════════════════════════════════════╗
// ║  LOOP — runs forever                                                      ║
// ╚════════════════════════════════════════════════════════════════════════════╝

void loop()
{
  lv_timer_handler();
  screensLoop();
  delay(1);
}
