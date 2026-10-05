#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <NimBLEDevice.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define DHTPIN 4
#define DHTTYPE DHT11

static constexpr char SERVICE_UUID[] = "6e7bdab4-e3c6-45ce-a0ef-f7e4d2c8ae45";
static constexpr char RX_UUID[] = "1e5d03d4-a534-4c81-bc9b-3056bf878d15";
static constexpr char TX_UUID[] = "44d2187d-f65e-4a55-b5fa-00da1726c6f1";

Adafruit_BMP280 bmp;
DHT dht(DHTPIN, DHTTYPE);

static NimBLEClient *bleClient = nullptr;
static NimBLERemoteCharacteristic *rxCharacteristic = nullptr;
static NimBLERemoteCharacteristic *txCharacteristic = nullptr;

static volatile bool isConnected = false;
static volatile bool requestPending = false;
static bool bmpAvailable = false;

static void readSensorsAndSend()
{
  float humidity = dht.readHumidity();
  float temperature = NAN;
  float pressure = NAN;

  if (bmpAvailable)
  {
    temperature = bmp.readTemperature();
    pressure = bmp.readPressure() / 100.0F;
  }

  if (isnan(temperature))
  {
    temperature = dht.readTemperature();
  }

  int hum = isnan(humidity) ? -1 : static_cast<int>(round(humidity));

  if (rxCharacteristic != nullptr && isConnected)
  {
    char payload[128];
    snprintf(
        payload,
        sizeof(payload),
        "ROOM:{\"temperature_c\":%.1f,\"humidity_percent\":%d,\"pressure_hpa\":%.1f}",
        temperature,
        hum,
        pressure);

    bool sent = rxCharacteristic->writeValue(payload, strlen(payload), false);
    Serial.printf("Transmitted: %s (%s)\n", payload, sent ? "OK" : "FAIL");
  }
}

static void onNotify(
    NimBLERemoteCharacteristic *chr,
    uint8_t *data,
    size_t length,
    bool isNotify)
{
  std::string msg(reinterpret_cast<char *>(data), length);
  Serial.printf("Display BLE call: %s\n", msg.c_str());

  if (msg.find("REFRESH_ROOM") != std::string::npos)
  {
    requestPending = true;
  }
}

class ClientCallbacks : public NimBLEClientCallbacks
{
  void onConnect(NimBLEClient *client) override
  {
    isConnected = true;
    Serial.println("Connected to Round-Display");
  }

  void onDisconnect(NimBLEClient *client, int reason) override
  {
    isConnected = false;
    rxCharacteristic = nullptr;
    txCharacteristic = nullptr;
    Serial.printf("Disconnected (reason: %d)\n", reason);
  }
};

static ClientCallbacks clientCallbacks;

static bool connectToDisplay()
{
  Serial.println("Scanning for Round-Display...");
  NimBLEScan *scan = NimBLEDevice::getScan();
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(99);

  NimBLEScanResults results = scan->getResults(3000);

  NimBLEAddress foundAddress;
  bool found = false;

  for (size_t i = 0; i < results.getCount(); i++)
  {
    const NimBLEAdvertisedDevice *dev = results.getDevice(i);
    if (dev->isAdvertisingService(NimBLEUUID(SERVICE_UUID)) || dev->getName() == "Round-Display")
    {
      foundAddress = dev->getAddress();
      found = true;
      break;
    }
  }

  scan->clearResults();

  if (!found)
  {
    Serial.println("Round-Display not in range");
    return false;
  }

  Serial.println("Found Round-Display, connecting...");
  if (bleClient == nullptr)
  {
    bleClient = NimBLEDevice::createClient();
    bleClient->setClientCallbacks(&clientCallbacks);
  }

  if (!bleClient->connect(foundAddress))
  {
    Serial.println("Connection failed");
    return false;
  }

  NimBLERemoteService *service = bleClient->getService(SERVICE_UUID);
  if (service == nullptr)
  {
    Serial.println("Service not found");
    bleClient->disconnect();
    return false;
  }

  rxCharacteristic = service->getCharacteristic(RX_UUID);
  txCharacteristic = service->getCharacteristic(TX_UUID);

  if (rxCharacteristic == nullptr || txCharacteristic == nullptr)
  {
    Serial.println("Characteristics not found");
    bleClient->disconnect();
    return false;
  }

  if (txCharacteristic->canNotify())
  {
    txCharacteristic->subscribe(true, onNotify);
  }

  bleClient->updateConnParams(800, 1600, 4, 600);

  readSensorsAndSend();
  return true;
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting Room-Sensor BLE...");

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setTimeOut(100);
  dht.begin();

  bmpAvailable = bmp.begin(0x76);
  if (!bmpAvailable)
  {
    bmpAvailable = bmp.begin(0x77);
  }

  Serial.printf("BMP280 %s\n", bmpAvailable ? "detected" : "not found");

  NimBLEDevice::init("Room-Sensor");
}

void loop()
{
  if (!isConnected)
  {
    if (!connectToDisplay())
    {
      delay(2000);
      return;
    }
  }

  if (requestPending)
  {
    requestPending = false;
    readSensorsAndSend();
  }

  delay(50);
}