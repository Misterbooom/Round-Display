#include "utils.h"
#include "secrets.h"
#include "WiFi.h"

namespace Utils
{

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
