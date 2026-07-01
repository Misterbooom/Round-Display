#include "ScreenManager.h"
#include <lvgl.h>

static lv_obj_t *_screens[SCREEN_COUNT] = {nullptr};

static Screen _current = SCREEN_CLOCK;

static bool _switching = false;

static void onScreenLoaded(lv_event_t *e)
{
  (void)e;
  _switching = false;
}

void screensInit()
{
  _screens[SCREEN_CLOCK] = ClockScreen::create();
  _screens[SCREEN_WEATHER] = WeatherScreen::create();
  _screens[SCREEN_SETTINGS] = SettingsScreen::create();

  for (int i = 0; i < SCREEN_COUNT; i++)
  {
    if (_screens[i] != nullptr)
    {
      lv_obj_add_event_cb(
          _screens[i],
          onScreenLoaded,
          LV_EVENT_SCREEN_LOADED,
          nullptr);
    }
  }

  lv_screen_load(_screens[SCREEN_CLOCK]);
  _current = SCREEN_CLOCK;
}

void screensSwitch(Screen s, bool forward)
{
  if (s >= SCREEN_COUNT)
    return;

  if (_screens[s] == nullptr)
    return;

  if (s == _current)
    return;

  if (_switching)
    return;

  _switching = true;
  _current = s;
  lv_screen_load_anim(
      _screens[s],

      forward
          ? LV_SCR_LOAD_ANIM_OVER_LEFT
          : LV_SCR_LOAD_ANIM_OVER_RIGHT,

      280,
      0,
      false);
}

void nextScreen()
{
  if (_current >= SCREEN_COUNT - 1)
  {
    screensSwitch(SCREEN_CLOCK, true);
    return;
  }

  screensSwitch(static_cast<Screen>(_current + 1), true);
}

void previousScreen()
{
  if (_current <= SCREEN_CLOCK)
  {
    screensSwitch(
        static_cast<Screen>(SCREEN_COUNT - 1),
        false);
    return;
  }

  screensSwitch(static_cast<Screen>(_current - 1), false);
}

void screensLoop()
{
  if (_switching)
    return;

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

Screen screensCurrent()
{
  return _current;
}