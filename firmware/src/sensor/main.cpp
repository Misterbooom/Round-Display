#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_sleep.h>
#include <cstring>

#define I2C_SDA 21
#define I2C_SCL 22
#define DHTPIN 4
#define DHTTYPE DHT11

static constexpr uint64_t SLEEP_DURATION_SEC = 120ULL;
static constexpr uint8_t ESPNOW_CHANNEL = 1;
static constexpr uint8_t ESPNOW_BROADCAST[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

Adafruit_BMP280 bmp;
DHT dht(DHTPIN, DHTTYPE);

struct SensorReading
{
  float temperature = NAN;
  int humidity = -1;
  float pressure = NAN;
};

static SensorReading readSensors()
{
  SensorReading reading;

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setTimeOut(100);

  dht.begin();
  delay(1000);

  bool bmpFound = bmp.begin(0x76);
  if (!bmpFound)
  {
    bmpFound = bmp.begin(0x77);
  }

  if (bmpFound)
  {
    reading.temperature = bmp.readTemperature();
    reading.pressure = bmp.readPressure() / 100.0F;
  }

  float dhtHum = dht.readHumidity();
  if (!isnan(dhtHum))
  {
    reading.humidity = static_cast<int>(round(dhtHum));
  }

  if (isnan(reading.temperature))
  {
    reading.temperature = dht.readTemperature();
  }

  Serial.printf(
      "[Sensor] T: %.1f C | H: %d%% | P: %.1f hPa\n",
      reading.temperature,
      reading.humidity,
      reading.pressure);

  return reading;
}

static void broadcastStatsAndSleep(const SensorReading &data)
{
  char jsonPayload[96];
  snprintf(
      jsonPayload,
      sizeof(jsonPayload),
      "{\"t\":%.1f,\"h\":%d,\"p\":%.1f}",
      isnan(data.temperature) ? 0.0f : data.temperature,
      data.humidity,
      isnan(data.pressure) ? 0.0f : data.pressure);

  char fullPayload[128];
  snprintf(fullPayload, sizeof(fullPayload), "ROOM:%s", jsonPayload);

  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("[ESP-NOW] Initialization failed");
  }
  else
  {
    esp_now_peer_info_t peerInfo{};
    memcpy(peerInfo.peer_addr, ESPNOW_BROADCAST, sizeof(ESPNOW_BROADCAST));
    peerInfo.channel = ESPNOW_CHANNEL;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      Serial.println("[ESP-NOW] Failed to add broadcast peer");
    }
    else
    {
      Serial.println("[ESP-NOW] Sending room sensor data...");
      for (uint8_t attempt = 0; attempt < 3; ++attempt)
      {
        esp_err_t result = esp_now_send(ESPNOW_BROADCAST,
                                        reinterpret_cast<const uint8_t *>(fullPayload),
                                        strlen(fullPayload) + 1);
        Serial.printf("[ESP-NOW] Send %u: %s\n", attempt + 1, result == ESP_OK ? "queued" : "failed");
        delay(50);
      }
    }

    esp_now_deinit();
  }

  Serial.printf("[Power] Entering deep sleep for %llu seconds...\n", SLEEP_DURATION_SEC);
  Serial.flush();

  esp_sleep_enable_timer_wakeup(SLEEP_DURATION_SEC * 1000000ULL);
  esp_deep_sleep_start();
}

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Room-Sensor Wakeup ===");

  SensorReading data = readSensors();
  broadcastStatsAndSleep(data);
}

void loop()
{
}