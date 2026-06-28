#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <string>

namespace Ble
{
  inline constexpr char SERVICE_UUID[] = "6e7bdab4-e3c6-45ce-a0ef-f7e4d2c8ae45";
  inline constexpr char RX_UUID[] = "1e5d03d4-a534-4c81-bc9b-3056bf878d15";
  inline constexpr char TX_UUID[] = "44d2187d-f65e-4a55-b5fa-00da1726c6f1";
  inline constexpr char DEVICE_NAME[] = "Round-Display";

  inline NimBLECharacteristic *txCharacteristic = nullptr;
  inline NimBLEServer *server = nullptr;

  inline void handleCommand(String command)
  {
    command.trim();

    Serial.println("BLE: " + command);

    if (command.startsWith("TIME:"))
    {
      String time = command.substring(5);
      Serial.println("New time: " + time);
    }
    else if (command.startsWith("WEATHER:"))
    {
      String weather = command.substring(8);
      Serial.println("Weather: " + weather);
    }
    else if (command == "PING")
    {
      if (txCharacteristic)
      {
        txCharacteristic->setValue("PONG");
        txCharacteristic->notify();
      }
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