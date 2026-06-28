#include "ScreenManager.h"
#include "TouchScreen.h"
// Force linker to pull LVGL font symbols from static library.
// `volatile` prevents the linker from stripping these references.
// The fonts are defined as `const lv_font_t` in LVGL's C files;
// we use `extern` without `const` here to avoid a clangd false-positive.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

namespace ClockScreen
{
  lv_obj_t *create();
  void update();
  void destroy(lv_obj_t *);
  void createBatteryIcon(lv_obj_t *);
  void updateBatteryFill();
}

namespace WeatherScreen
{
  lv_obj_t *create();
  void update();
  void destroy(lv_obj_t *);
}

namespace SettingsScreen
{
  lv_obj_t *create();
  void update();
  void destroy(lv_obj_t *);
}

static lv_obj_t *_screens[SCREEN_COUNT] = {nullptr};
static Screen _current = SCREEN_CLOCK;

void screensInit()
{
  _screens[SCREEN_CLOCK] = ClockScreen::create();
  _screens[SCREEN_WEATHER] = WeatherScreen::create();
  _screens[SCREEN_SETTINGS] = SettingsScreen::create();

  lv_scr_load(_screens[SCREEN_CLOCK]);
}

void screensSwitch(Screen s)
{
  if (s >= SCREEN_COUNT || _screens[s] == nullptr)
    return;
  _current = s;
  lv_scr_load(_screens[s]);
}
void nextScreen()
{
  if (_current >= SCREEN_COUNT - 1)
  {
    screensSwitch(SCREEN_CLOCK);
    return;
  }
  screensSwitch(static_cast<Screen>(_current + 1));
}
void previousScreen()
{
  if (_current <= 0)
  {
    screensSwitch(static_cast<Screen>(SCREEN_COUNT - 1));
    return;
  }
  screensSwitch(static_cast<Screen>(_current - 1));
}
void screensLoop()
{
  switch (_current)
  {
  case SCREEN_CLOCK:
    ClockScreen::update();
    break;
  case SCREEN_WEATHER:
    WeatherScreen::update();
    break;
  case SCREEN_SETTINGS:
    SettingsScreen::update();
    break;
  default:
    break;
  }
}

Screen screensCurrent() { return _current; }
