#pragma once

#include <Arduino.h>
#include <functional>

namespace Ble
{
  using WeatherCallback = std::function<bool(const char *)>;
  using RoomCallback = std::function<bool(const char *)>;

  void init();
  void update();

  bool requestWeather();
  bool requestRoom();
  void sendBattery();

  void setWeatherCallback(WeatherCallback cb);
  void setRoomCallback(RoomCallback cb);
  bool isConnected();
}