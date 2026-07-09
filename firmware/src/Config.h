#pragma once

#include <Arduino.h>
#include <Preferences.h>

namespace Config
{
  struct Data
  {
    uint16_t brightness = 70;
    uint16_t timeoutSec = 30;

    bool use24Hour = true;
    bool showSeconds = false;
    bool animationsEnabled = true;

    bool useCelsius = true;
    uint16_t weatherUpdateMin = 30;
  };

  inline Data data;
  inline Preferences preferences;

  inline bool setup()
  {
    return preferences.begin("config", false);
  }

  inline void save()
  {
    preferences.putUShort("brightness", data.brightness);
    preferences.putUShort("timeoutSec", data.timeoutSec);

    preferences.putBool("use24Hour", data.use24Hour);
    preferences.putBool("showSeconds", data.showSeconds);
    preferences.putBool("animations", data.animationsEnabled);

    preferences.putBool("useCelsius", data.useCelsius);
    preferences.putUShort("weatherMin", data.weatherUpdateMin);
  }

  inline void load()
  {
    data.brightness =
        preferences.getUShort("brightness", 70);

    data.timeoutSec =
        preferences.getUShort("timeoutSec", 30);

    data.use24Hour =
        preferences.getBool("use24Hour", true);

    data.showSeconds =
        preferences.getBool("showSeconds", false);

    data.animationsEnabled =
        preferences.getBool("animations", true);

    data.useCelsius =
        preferences.getBool("useCelsius", true);

    data.weatherUpdateMin =
        preferences.getUShort("weatherMin", 30);
  }

  inline void reset()
  {
    preferences.clear();
    data = Data{};
  }
}