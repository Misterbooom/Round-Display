import asyncio
from datetime import datetime
from bleak import BleakScanner, BleakClient
import requests
DEVICE_NAME = "Round-Display"
RX_UUID = "1e5d03d4-a534-4c81-bc9b-3056bf878d15"
TX_UUID = "44d2187d-f65e-4a55-b5fa-00da1726c6f1"
SERVICE_UUID = "6e7bdab4-e3c6-45ce-a0ef-f7e4d2c8ae45"

def on_notification(_, data: bytearray):
    print("ESP:", data.decode("utf-8", errors="ignore"))
def get_weather(city: str) -> str:
    geo = requests.get(
        "https://geocoding-api.open-meteo.com/v1/search",
        params={
            "name": city,
            "count": 1,
            "language": "en",
            "format": "json",
        },
        timeout=10,
    )
    geo.raise_for_status()

    results = geo.json().get("results")
    if not results:
        raise RuntimeError(f"Город '{city}' не найден")

    place = results[0]
    latitude = place["latitude"]
    longitude = place["longitude"]
    city_name = place["name"]

    weather = requests.get(
        "https://api.open-meteo.com/v1/forecast",
        params={
            "latitude": latitude,
            "longitude": longitude,
            "current": (
                "temperature_2m,"
                "apparent_temperature,"
                "relative_humidity_2m,"
                "weather_code,"
                "wind_speed_10m"
            ),
            "wind_speed_unit": "kmh",
            "timezone": "auto",
        },
        timeout=10,
    )
    weather.raise_for_status()

    current = weather.json()["current"]

    temp = current["temperature_2m"]
    feels = current["apparent_temperature"]
    humidity = current["relative_humidity_2m"]
    code = current["weather_code"]
    wind = current["wind_speed_10m"]

    print(f"{city_name}: {temp}°C")

    return (
        f"WEATHER:"
        f"temp={temp};"
        f"feels={feels};"
        f"humidity={humidity};"
        f"code={code};"
        f"wind={wind};"
    )

async def main():
    print("Scanning for BLE devices (5s)...")
    all_devices = await BleakScanner.discover(timeout=5.0, return_adv=True)

    if not all_devices:
        print("❌ No BLE devices found at all — is PC Bluetooth on?")
        return

    print(f"Found {len(all_devices)} device(s):")
    for addr, (dev, adv) in all_devices.items():
        name = dev.name or adv.local_name or "(no name)"
        rssi = adv.rssi if adv else "?"
        print(f"  {addr}  RSSI={rssi: >4}  {name}")

    print(f"\nSearching for '{DEVICE_NAME}'...")
    device = await BleakScanner.find_device_by_filter(
        lambda dev, adv: SERVICE_UUID.lower() in [
            uuid.lower() for uuid in adv.service_uuids
        ],
        timeout=8.0
    )

    if device is None:
        print(f"❌ '{DEVICE_NAME}' not found in scan results")
        return

    async with BleakClient(device) as client:
        print("Connected:", client.is_connected)

        await client.start_notify(TX_UUID, on_notification)

        now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        first_sent = True
        while True:

            if first_sent:
                await client.write_gatt_char(
                    RX_UUID,
                    f"TIME:{now}".encode()
                )
                first_sent = False

            await client.write_gatt_char(
                RX_UUID,
                f"WEATHER:{get_weather("Gdansk")}"
            )
            await asyncio.sleep(60*30)


asyncio.run(main())