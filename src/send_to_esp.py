import asyncio
from datetime import datetime
from bleak import BleakScanner, BleakClient
import requests
import json

DEVICE_NAME = "Round-Display"
RX_UUID = "1e5d03d4-a534-4c81-bc9b-3056bf878d15"
TX_UUID = "44d2187d-f65e-4a55-b5fa-00da1726c6f1"
SERVICE_UUID = "6e7bdab4-e3c6-45ce-a0ef-f7e4d2c8ae45"

def on_notification(_, data: bytearray):
    print("ESP:", data.decode("utf-8", errors="ignore"))

def get_weather(city: str) -> str:
    try:
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
            raise RuntimeError(f"City '{city}' not found")

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

        payload = {
            "city": city_name,
            "latitude": latitude,
            "longitude": longitude,
            "temperature_c": temp,
            "feels_like_c": feels,
            "humidity_percent": humidity,
            "weather_code": code,
            "wind_kmh": wind,
            "fetched_at": datetime.utcnow().isoformat() + "Z",
        }

        return json.dumps(payload, separators=(",", ":"))
    except Exception as e:
        print(f"Error fetching weather: {e}")
        return ""

async def main():
    while True:
        try:
            print(f"\nSearching for '{DEVICE_NAME}' via Service UUID...")
            device = await BleakScanner.find_device_by_filter(
                lambda dev, adv: SERVICE_UUID.lower() in [
                    uuid.lower() for uuid in adv.service_uuids
                ],
                timeout=10.0
            )

            if device is None:
                print(f"'{DEVICE_NAME}' not found. Retrying in 5 seconds...")
                await asyncio.sleep(5)
                continue

            print(f"Connecting to {device.address}...")
            async with BleakClient(device) as client:
                print("Connected:", client.is_connected)

                await client.start_notify(TX_UUID, on_notification)

                first_sent = True
                while client.is_connected:
                    if first_sent:
                        now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                        print("Sending initial TIME and WEATHER data...")
                        await client.write_gatt_char(RX_UUID, f"TIME:{now}".encode())
                        
                        weather_data = get_weather('Gdansk')
                        if weather_data:
                            await client.write_gatt_char(RX_UUID, f"WEATHER:{weather_data}".encode())
                        
                        first_sent = False
                    else:
                        weather_data = get_weather('Gdansk')
                        if weather_data:
                            print("Sending updated WEATHER data...")
                            await client.write_gatt_char(RX_UUID, f"WEATHER:{weather_data}".encode())

                    await asyncio.sleep(60 * 30)

        except Exception as e:
            print(f"Connection lost or error occurred: {e}")
            print("Reconnecting in 5 seconds...")
            await asyncio.sleep(5)

asyncio.run(main())