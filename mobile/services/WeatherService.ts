export async function getWeather(city: string): Promise<string> {
	try {
		const geoRes = await fetch(
			`https://geocoding-api.open-meteo.com/v1/search?name=${encodeURIComponent(city)}&count=1&language=en&format=json`
		)
		
		if (!geoRes.ok) {
			throw new Error(`Geo API error: ${geoRes.status}`)
		}

		const geoData = await geoRes.json()
		const results = geoData.results

		if (!results || results.length === 0) {
			throw new Error(`City '${city}' not found`)
		}

		const place = results[0]
		const latitude = place.latitude
		const longitude = place.longitude
		const cityName = place.name

		const weatherRes = await fetch(
			`https://api.open-meteo.com/v1/forecast?latitude=${latitude}&longitude=${longitude}&current=temperature_2m,apparent_temperature,relative_humidity_2m,weather_code,wind_speed_10m&wind_speed_unit=kmh&timezone=auto`
		)

		if (!weatherRes.ok) {
			throw new Error(`Weather API error: ${weatherRes.status}`)
		}

		const weatherData = await weatherRes.json()
		const current = weatherData.current

		const temp = current.temperature_2m
		const feels = current.apparent_temperature
		const humidity = current.relative_humidity_2m
		const code = current.weather_code
		const wind = current.wind_speed_10m

		const payload = {
			city: cityName,
			latitude,
			longitude,
			temperature_c: temp,
			feels_like_c: feels,
			humidity_percent: humidity,
			weather_code: code,
			wind_kmh: wind,
			fetched_at: new Date().toISOString().replace('Z', '') + 'Z',
		}

		return JSON.stringify(payload)
	} catch (e) {
		return ""
	}
}
