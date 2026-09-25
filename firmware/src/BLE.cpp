#include "BLE.h"

#include <NimBLEDevice.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <atomic>
#include <string>
#include <utils.h>
#include "Config.h"

namespace Ble
{
  namespace
  {
    constexpr char SERVICE_UUID[] = "6e7bdab4-e3c6-45ce-a0ef-f7e4d2c8ae45";
    constexpr char RX_UUID[] = "1e5d03d4-a534-4c81-bc9b-3056bf878d15";
    constexpr char TX_UUID[] = "44d2187d-f65e-4a55-b5fa-00da1726c6f1";
    constexpr char DEVICE_NAME[] = "Round-Display";

    constexpr uint16_t BLE_MTU_SIZE = 512;
    constexpr size_t CMD_QUEUE_LEN = 8;
    constexpr size_t CMD_MAX_SIZE = 512;
    constexpr uint32_t INITIAL_SYNC_DELAY_MS = 2000UL;
    constexpr uint32_t WEATHER_RETRY_DELAY_MS = 5000UL;

    struct CommandMsg
    {
      char payload[CMD_MAX_SIZE];
    };

    NimBLEServer *server = nullptr;
    NimBLECharacteristic *txCharacteristic = nullptr;
    QueueHandle_t commandQueue = nullptr;
    WeatherCallback onWeatherReceive = nullptr;

    std::atomic<bool> clientSubscribed{false};
    std::atomic<bool> pendingInitialSync{false};
    std::atomic<uint32_t> syncDelayStartMs{0};

    uint32_t lastWeatherRequestMs = 0;
    uint32_t lastBatterySendMs = 0;
    uint32_t weatherRetryAtMs = 0;

    bool sendNotification(const char *data)
    {
      if (!isConnected() || txCharacteristic == nullptr)
      {
        return false;
      }
      txCharacteristic->setValue(reinterpret_cast<const uint8_t *>(data), strlen(data));
      return txCharacteristic->notify();
    }

    void processCommand(String &command)
    {
      command.trim();
      Serial.println("BLE: " + command);

      if (command.startsWith("TIME:"))
      {
        Utils::setDate(command.c_str() + 5);
      }
      else if (command.startsWith("WEATHER:"))
      {
        const char *json = command.c_str() + 8;
        Serial.printf("Weather: %s\n", json);
        if (onWeatherReceive)
        {
          onWeatherReceive(json);
        }
      }
      else if (command == "PING")
      {
        sendNotification("PONG");
      }
      else if (command.startsWith("brightness:"))
      {
        char *endPtr = nullptr;
        long val = strtol(command.c_str() + 11, &endPtr, 10);
        if (endPtr != command.c_str() + 11)
        {
          Config::data.brightness = constrain(val, 1L, 255L);
          Serial.printf("[BLE] Setting brightness to %d\n", Config::data.brightness);
        }
      }
    }

    class RxCallbacks : public NimBLECharacteristicCallbacks
    {
    public:
      void onWrite(NimBLECharacteristic *characteristic, NimBLEConnInfo &connInfo) override
      {
        std::string value = characteristic->getValue();
        if (value.empty() || commandQueue == nullptr)
        {
          return;
        }

        CommandMsg msg{};
        strncpy(msg.payload, value.c_str(), sizeof(msg.payload) - 1);
        xQueueSend(commandQueue, &msg, 0);
      }
    };

    class TxCallbacks : public NimBLECharacteristicCallbacks
    {
    public:
      void onSubscribe(NimBLECharacteristic *characteristic, NimBLEConnInfo &connInfo, uint16_t subValue) override
      {
        if (subValue & 0x01)
        {
          syncDelayStartMs.store(millis());
          pendingInitialSync.store(true);
          clientSubscribed.store(true);
          Serial.println("[BLE] Client subscribed to TX");
        }
        else
        {
          clientSubscribed.store(false);
          pendingInitialSync.store(false);
          Serial.println("[BLE] Client unsubscribed from TX");
        }
      }
    };

    class ServerCallbacks : public NimBLEServerCallbacks
    {
    public:
      void onConnect(NimBLEServer *srv, NimBLEConnInfo &connInfo) override
      {
        Serial.println("[BLE] Device connected");
      }

      void onDisconnect(NimBLEServer *srv, NimBLEConnInfo &connInfo, int reason) override
      {
        if (srv->getConnectedCount() == 0)
        {
          clientSubscribed.store(false);
          pendingInitialSync.store(false);
        }
        Serial.println("[BLE] Device disconnected");
      }
    };

    RxCallbacks rxCallbacks;
    TxCallbacks txCallbacks;
    ServerCallbacks serverCallbacks;
  }

  void setWeatherCallback(WeatherCallback cb)
  {
    onWeatherReceive = cb;
  }

  bool isConnected()
  {
    return server != nullptr && server->getConnectedCount() > 0 && clientSubscribed.load();
  }

  bool requestWeather()
  {
    if (!isConnected())
    {
      Serial.println("[BLE] Weather request skipped: client not ready");
      return false;
    }

    bool sent = sendNotification("REFRESH_WEATHER");
    if (sent)
    {
      lastWeatherRequestMs = millis();
      weatherRetryAtMs = 0;
    }

    Serial.printf("[BLE] Weather request: %s\n", sent ? "sent" : "failed");
    return sent;
  }

  void sendBattery()
  {
    if (!isConnected())
    {
      return;
    }

    int pct = Utils::batteryPercent();
    if (pct < 0)
    {
      return;
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "battery:%d%%", pct);
    Serial.printf("[Sending battery]: %s\n", buf);
    sendNotification(buf);
  }

  void update()
  {
    if (commandQueue != nullptr)
    {
      CommandMsg msg;
      while (xQueueReceive(commandQueue, &msg, 0) == pdTRUE)
      {
        String cmd(msg.payload);
        processCommand(cmd);
      }
    }

    if (!isConnected())
    {
      return;
    }

    uint32_t now = millis();

    if (pendingInitialSync.load())
    {
      if (now - syncDelayStartMs.load() >= INITIAL_SYNC_DELAY_MS)
      {
        pendingInitialSync.store(false);

        sendBattery();
        lastBatterySendMs = now;

        if (Config::data.weatherUpdateMin > 0 && !requestWeather())
        {
          weatherRetryAtMs = now + WEATHER_RETRY_DELAY_MS;
        }
      }
      return;
    }

    if (Config::data.weatherUpdateMin > 0)
    {
      uint32_t intervalMs = static_cast<uint32_t>(Config::data.weatherUpdateMin) * 60000UL;
      bool retryDue = (weatherRetryAtMs != 0) && (static_cast<int32_t>(now - weatherRetryAtMs) >= 0);
      bool intervalDue = (weatherRetryAtMs == 0) && (now - lastWeatherRequestMs >= intervalMs);

      if (retryDue || intervalDue)
      {
        if (!requestWeather())
        {
          weatherRetryAtMs = now + WEATHER_RETRY_DELAY_MS;
        }
      }
    }

    if (Config::data.batteryUpdateMin > 0)
    {
      uint32_t batteryIntervalMs = static_cast<uint32_t>(Config::data.batteryUpdateMin) * 60000UL;
      if (now - lastBatterySendMs >= batteryIntervalMs)
      {
        sendBattery();
        lastBatterySendMs = now;
      }
    }
  }

  void init()
  {
    if (server != nullptr)
    {
      Serial.println("[BLE] Already initialized, skipping.");
      return;
    }

    if (commandQueue == nullptr)
    {
      commandQueue = xQueueCreate(CMD_QUEUE_LEN, sizeof(CommandMsg));
    }

    NimBLEDevice::init(DEVICE_NAME);
    NimBLEDevice::setMTU(BLE_MTU_SIZE);

    NimBLEServer *srv = NimBLEDevice::createServer();
    srv->setCallbacks(&serverCallbacks);
    srv->advertiseOnDisconnect(true);

    NimBLEService *service = srv->createService(SERVICE_UUID);

    NimBLECharacteristic *rxCharacteristic = service->createCharacteristic(
        RX_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
    rxCharacteristic->setCallbacks(&rxCallbacks);

    txCharacteristic = service->createCharacteristic(
        TX_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    txCharacteristic->setCallbacks(&txCallbacks);

    if (!srv->start())
    {
      Serial.println("[BLE] Failed to start GATT server");
      return;
    }

    server = srv;

    NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->enableScanResponse(true);
    advertising->setName(DEVICE_NAME);
    advertising->start();

    Serial.println("[BLE] Init done!");
  }
}