#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include "BLE.h"
#include "Config.h"
#include "ScreenManager.h"

namespace Callbacks
{
  inline bool isChecked(lv_event_t *event)
  {
    lv_obj_t *control =
        static_cast<lv_obj_t *>(lv_event_get_current_target(event));
    return control != nullptr &&
           lv_obj_has_state(control, LV_STATE_CHECKED);
  }

  inline int32_t sliderValue(lv_event_t *event)
  {
    lv_obj_t *slider =
        static_cast<lv_obj_t *>(lv_event_get_current_target(event));
    return slider != nullptr ? lv_slider_get_value(slider) : 0;
  }

  inline void timeFormat_cb(lv_event_t *event)
  {
    Config::data.use24Hour = isChecked(event);
    Config::save();
    Serial.printf("[Settings] Time format: %s\n",
                  Config::data.use24Hour ? "24 H" : "12 H");
  }

  inline void temperatureUnit_cb(lv_event_t *event)
  {
    Config::data.useCelsius = !isChecked(event);
    Config::save();
    WeatherScreen::refresh();
    Serial.printf("[Settings] Temperature unit: %s\n",
                  Config::data.useCelsius ? "C" : "F");
  }

  inline void weatherRefreshRate_cb(lv_event_t *event)
  {
    Config::data.weatherUpdateMin =
        static_cast<uint16_t>(sliderValue(event));
    Config::save();
    Ble::resetWeatherRequestTimer();
    Serial.printf("[Settings] Weather refresh: %u min\n",
                  Config::data.weatherUpdateMin);
  }

  inline void refreshWeather_cb(lv_event_t *)
  {
    Ble::requestWeather();
  }

  inline void screenTimeout_cb(lv_event_t *event)
  {
    Config::data.timeoutSec = static_cast<uint16_t>(sliderValue(event));
    Config::save();
    Serial.printf("[Settings] Screen timeout: %u s\n",
                  Config::data.timeoutSec);
  }

  inline void alwaysOn_cb(lv_event_t *event)
  {
    static uint16_t previousTimeoutSec = 30;

    if (isChecked(event))
    {
      if (Config::data.timeoutSec > 0)
      {
        previousTimeoutSec = Config::data.timeoutSec;
      }
      Config::data.timeoutSec = 0;
    }
    else if (Config::data.timeoutSec == 0)
    {
      Config::data.timeoutSec = previousTimeoutSec;
    }

    Config::save();
    Serial.printf("[Settings] Always on: %s\n",
                  Config::data.timeoutSec == 0 ? "enabled" : "disabled");
  }
}
