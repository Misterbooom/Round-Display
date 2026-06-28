#pragma once
#include "Arduino.h"
#include "WiFi.h"
#include "secrets.h"
#include <lvgl.h>
#include "RTClib.h"
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

namespace Utils
{
  static RTC_PCF8563 rtc;
  static bool rtcInitialized;
  int batteryPercent();
  bool initRTC();
  template <typename Func>
  bool initWifi(Func callback)
  {

    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.disconnect(true, true);
    delay(1000);

    WiFi.setSleep(false);
    delay(100);

    int n = WiFi.scanNetworks();
    Serial.printf("WiFi scan found %d networks\n", n);
    for (int i = 0; i < n; i++)
    {
      Serial.println(WiFi.SSID(i));
    }

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.printf("\nConnecting to %s ...\n", WIFI_SSID);

    uint8_t tries = 0;

    while (WiFi.status() != WL_CONNECTED)
    {
      if (tries >= 10)
      {
        int status = WiFi.status();
        Serial.printf("\nWiFi connect failed — status: %d", status);
        if (status == WL_DISCONNECTED)
          Serial.print(" (WL_DISCONNECTED — is this a 2.4 GHz network? ESP32-S3 has no 5 GHz radio)");
        if (status == WL_CONNECT_FAILED)
          Serial.print(" (WL_CONNECT_FAILED — check SSID/password)");
        if (status == WL_NO_SSID_AVAIL)
          Serial.print(" (WL_NO_SSID_AVAIL — SSID not found)");
        Serial.println();
        return false;
      }
      Serial.print(".");
      Serial.printf(" Status: %d", WiFi.status());
      tries++;
      callback(tries);

      for (int d = 0; d < 500; d += 5)
      {
        lv_tick_inc(5);
        lv_timer_handler();
        delay(5);
      }
    }

    Serial.printf("\nWiFi connected! RSSI: %d dBm, IP: %s\n",
                  WiFi.RSSI(), WiFi.localIP().toString().c_str());

    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
      DateTime dt(
          (uint16_t)(timeinfo.tm_year + 1900),
          (uint8_t)(timeinfo.tm_mon + 1),
          (uint8_t)timeinfo.tm_mday,
          (uint8_t)timeinfo.tm_hour,
          (uint8_t)timeinfo.tm_min,
          (uint8_t)timeinfo.tm_sec);
      rtc.adjust(dt);
    }
    return true;
  }
  DateTime getDateTime();

}
