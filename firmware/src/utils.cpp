#include "utils.h"
#include "secrets.h"
#include "WiFi.h"

namespace Utils
{
  RTC_PCF8563 rtc;
  bool rtcInitialized = false;
  static portMUX_TYPE pendingDateMutex = portMUX_INITIALIZER_UNLOCKED;
  static DateTime pendingDate;
  static bool datePending = false;

  int batteryPercent()
  {
    int adc = analogReadMilliVolts(1);

    if (adc < 1500)
      return -1;

    int pct = ((adc - 1550) * 100) / (2100 - 1550);

    if (pct < 0)
      pct = 0;
    if (pct > 100)
      pct = 100;
    return pct;
  }

  bool initRTC()
  {
    if (!rtc.begin())
    {
      rtcInitialized = false;
      Serial.println("RTC not found");
      return false;
    }

    rtcInitialized = true;
    Serial.println("RTC initialized");
    return true;
  }

  void setDate(const char *date)
  {
    if (date == nullptr)
    {
      Serial.println("Invalid date: null");
      return;
    }

    DateTime dt(date);
    if (!dt.isValid())
    {
      Serial.printf("Invalid date: %s\n", date);
      return;
    }

    portENTER_CRITICAL(&pendingDateMutex);
    pendingDate = dt;
    datePending = true;
    portEXIT_CRITICAL(&pendingDateMutex);
  }

  void setDate(DateTime dt)
  {
    if (!rtcInitialized)
    {
      Serial.println("Cannot set date: RTC is not initialized");
      return;
    }

    rtc.adjust(dt);
  }

  void applyPendingDate()
  {
    DateTime dt;
    bool shouldAdjust;

    portENTER_CRITICAL(&pendingDateMutex);
    shouldAdjust = datePending;
    if (shouldAdjust)
    {
      dt = pendingDate;
      datePending = false;
    }
    portEXIT_CRITICAL(&pendingDateMutex);

    if (shouldAdjust)
    {
      setDate(dt);
    }
  }

  DateTime getDateTime()
  {
    if (rtcInitialized)
    {
      return rtc.now();
    }
    else
    {
      return DateTime();
    }
  }

} // namespace Utils
