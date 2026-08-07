#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <string>
#include <ArduinoJson.h>
#include <utils.h>
#include "Config.h"
#include "Screens/ScreenManager.h"
namespace Ble
{
  inline constexpr char SERVICE_UUID[] = "6e7bdab4-e3c6-45ce-a0ef-f7e4d2c8ae45";
  inline constexpr char RX_UUID[] = "1e5d03d4-a534-4c81-bc9b-3056bf878d15";
  inline constexpr char TX_UUID[] = "44d2187d-f65e-4a55-b5fa-00da1726c6f1";
  inline constexpr char DEVICE_NAME[] = "Round-Display";

  inline NimBLECharacteristic *txCharacteristic = nullptr;
  inline NimBLEServer *server = nullptr;
  inline std::function<bool(const char *)> onWeatherReceive = nullptr;
  inline uint32_t lastWeatherRequestMs = 0;
  inline uint32_t lastBatterySendMs = 0;
  inline bool pendingInitialBattery = false;
  inline uint32_t batteryDelayStartMs = 0;

  inline bool requestWeather()
  {
    if (server == nullptr || txCharacteristic == nullptr ||
        server->getConnectedCount() == 0)
    {
      Serial.println("[BLE] Weather request skipped: no client");
      return false;
    }

    txCharacteristic->setValue("REFRESH_WEATHER");
    bool sent = txCharacteristic->notify();
    if (sent)
    {
      lastWeatherRequestMs = millis();
    }

    Serial.printf("[BLE] Weather request: %s\n", sent ? "sent" : "failed");
    return sent;
  }

  inline void resetWeatherRequestTimer()
  {
    lastWeatherRequestMs = millis();
  }

  inline void sendBattery()
  {
    if (server == nullptr || txCharacteristic == nullptr ||
        server->getConnectedCount() == 0)
    {
      return;
    }

    int pct = Utils::batteryPercent();
    if (pct < 0)
      return; // invalid battery reading

    char buf[32];
    snprintf(buf, sizeof(buf), "battery:%d%%", pct);
    Serial.printf("[Sending battery]: %s", &buf);
    txCharacteristic->setValue(buf);
    txCharacteristic->notify();
  }

  inline void update()
  {
    if (server == nullptr || server->getConnectedCount() == 0)
    {
      return;
    }

    if (Config::data.weatherUpdateMin > 0)
    {
      uint32_t intervalMs =
          static_cast<uint32_t>(Config::data.weatherUpdateMin) * 60UL * 1000UL;
      if (millis() - lastWeatherRequestMs >= intervalMs)
      {
        requestWeather();
      }
    }

    if (pendingInitialBattery && (millis() - batteryDelayStartMs >= 2000))
    {
      sendBattery();
      lastBatterySendMs = millis();
      pendingInitialBattery = false;
    }
    else if (Config::data.batteryUpdateMin > 0)
    {
      uint32_t batteryIntervalMs =
          static_cast<uint32_t>(Config::data.batteryUpdateMin) * 60UL * 1000UL;
      if (millis() - lastBatterySendMs >= batteryIntervalMs)
      {
        sendBattery();
        lastBatterySendMs = millis();
      }
    }
  }

  inline void handleCommand(String command)
  {
    command.trim();

    Serial.println("BLE: " + command);

    if (command.startsWith("TIME:"))
    {
      const char *time = command.c_str() + 5;
      Utils::setDate(time);
    }
    else if (command.startsWith("WEATHER:"))
    {
      const char *json = command.c_str() + 8;

      Serial.print("Weather: ");
      Serial.println(json);

      if (onWeatherReceive)
      {
        onWeatherReceive(json);
      }
    }
    else if (command == "PING")
    {
      if (txCharacteristic)
      {
        txCharacteristic->setValue("PONG");
        txCharacteristic->notify();
      }
    }
    else if (command.startsWith("brightness:"))
    {
      const char *json = command.c_str() + 11;
      Config::data.brightness = atoi(json);
      Serial.printf("[BLE] Setting brightness to %d\n", Config::data.brightness, json);
    }
  }

  class RxCallbacks : public NimBLECharacteristicCallbacks
  {
  public:
    void onWrite(
        NimBLECharacteristic *characteristic,
        NimBLEConnInfo &connInfo) override
    {
      std::string value = characteristic->getValue();

      if (!value.empty())
        handleCommand(String(value.c_str()));
    }
  };

  class ServerCallbacks : public NimBLEServerCallbacks
  {
  public:
    void onConnect(
        NimBLEServer *server,
        NimBLEConnInfo &connInfo) override
    {
      resetWeatherRequestTimer();
      pendingInitialBattery = true;
      batteryDelayStartMs = millis();
      Serial.println("PC connected");
    }

    void onDisconnect(
        NimBLEServer *server,
        NimBLEConnInfo &connInfo,
        int reason) override
    {
      Serial.println("PC disconnected");
    }
  };

  inline void init()
  {
    if (server != nullptr)
    {
      Serial.println("[BLE] Already initialized, skipping.");
      return;
    }

    Serial.println("[BLE] Init — starting…");
    Serial.printf("[BLE]   Device name: %s\n", DEVICE_NAME);
    Serial.printf("[BLE]   Service UUID: %s\n", SERVICE_UUID);

    NimBLEDevice::init(DEVICE_NAME);
    Serial.println("[BLE]   NimBLEDevice::init() OK");

    server = NimBLEDevice::createServer();
    Serial.printf("[BLE]   createServer() → %p\n", server);
    server->setCallbacks(new ServerCallbacks());
    Serial.println("[BLE]   ServerCallbacks registered");

    server->advertiseOnDisconnect(true);

    NimBLEService *service = server->createService(SERVICE_UUID);
    Serial.printf("[BLE]   createService() → %p\n", service);

    NimBLECharacteristic *rxCharacteristic =
        service->createCharacteristic(
            RX_UUID,
            NIMBLE_PROPERTY::WRITE |
                NIMBLE_PROPERTY::WRITE_NR);

    rxCharacteristic->setCallbacks(new RxCallbacks());
    Serial.printf("[BLE]   RX characteristic created (%p)\n", rxCharacteristic);

    txCharacteristic = service->createCharacteristic(
        TX_UUID,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY);

    Serial.printf("[BLE]   TX characteristic created (%p)\n", txCharacteristic);

    if (!server->start())
    {
      Serial.println("[BLE] Failed to start GATT server");
      return;
    }

    Serial.println("[BLE]   GATT server started");

    NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();

    advertising->addServiceUUID(SERVICE_UUID);

    // UUID остаётся в advertising packet,
    // имя уйдёт в scan response
    advertising->enableScanResponse(true);
    advertising->setName(DEVICE_NAME);

    bool started = advertising->start();

    Serial.printf(
        "[BLE] Advertising started: %s\n",
        started ? "YES" : "NO");

    Serial.println("[BLE] Init — done!");
  }
}
