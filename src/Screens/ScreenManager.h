/*
 * Screen manager — owns screen switching and lifecycle.
 *
 * Usage:
 *   screensInit();                  // call once in setup()
 *   screensSwitch(SCREEN_CLOCK);    // switch anywhere
 *   screensLoop();                  // call in loop() for periodic updates
 */

#pragma once
#include <lvgl.h>

// ── List your screens here ──────────────────────────────────────────────────
enum Screen : uint8_t
{
  SCREEN_CLOCK,
  SCREEN_WEATHER,
  SCREEN_SETTINGS,
  SCREEN_COUNT
};

void screensInit();

void screensSwitch(Screen s);

void screensLoop();
void nextScreen();
void previousScreen();

Screen screensCurrent();
