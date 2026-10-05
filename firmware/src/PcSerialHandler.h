#include <Arduino.h>
namespace PcSerialHandler
{
  void update()
  {
    if (Serial.available() > 0)
    {
      char buf[64];

      size_t len = Serial.readBytesUntil('\n', buf, sizeof(buf) - 1);
      buf[len] = '\0';

      if (len > 0 && buf[len - 1] == '\r')
      {
        buf[len - 1] = '\0';
      }

      if (strcmp(buf, "PING") == 0)
      {
        Serial.println("PONG");
      }
      else if (strncmp(buf, "PC_TELEMETRY_", 13) == 0)
      {
        float load = 0.0f;
        float temp = 0.0f;

        if (sscanf(buf + 17, "%f, %f", &load, &temp) == 2)
        {
          if (strncmp(buf + 13, "GPU:", 4) == 0)
          {
            // Serial.printf("GPU: temp: %f, load: %f", load, temp);
            PcStatsScreen::setGpuTelemetry(load, temp);
          }
          else if (strncmp(buf + 13, "CPU:", 4) == 0)
          {
            // Serial.printf("CPU: temp: %f, load: %f", load, temp);
            PcStatsScreen::setCpuTelemetry(load, temp);
          }
        }
      }
    }
  }
}