/*
 * Screen manager — owns screen switching and lifecycle.
 *
 * Usage:
 *   screensInit();                  // call once in setup()
 *   screensSwitch(SCREEN_CLOCK);    // switch anywhere
 *   screensLoop();                  // call in loop() for periodic updates
 */

#pragma once
#include <Arduino.h>
#include <lvgl.h>
#include <string>
#include <ArduinoJson.h>
// ── List your screens here ──────────────────────────────────────────────────
enum Screen : uint8_t
{
  SCREEN_CLOCK,
  SCREEN_WEATHER,
  SCREEN_SETTINGS,
  SCREEN_COUNT
};
namespace ClockScreen
{
  lv_obj_t *create();
  void update();
}

namespace WeatherScreen
{
  lv_obj_t *create();
  void update();
  bool updateWeather(const char *json);
}

namespace SettingsScreen
{
  lv_obj_t *create();
  void update();
}

void screensInit();

void screensSwitch(Screen s, bool forward = true);

void screensLoop();
void nextScreen();
void previousScreen();

Screen screensCurrent();
