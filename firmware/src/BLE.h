#pragma once

#include <Arduino.h>
#include <functional>

namespace Ble
{
  using WeatherCallback = std::function<bool(const char *)>;

  void init();
  void update();

  bool requestWeather();
  void sendBattery();

  void setWeatherCallback(WeatherCallback cb);
  bool isConnected();
}