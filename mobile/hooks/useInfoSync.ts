import { bleService } from '@/services/BLEService'
import { StorageService } from '@/services/StorageService'
import { getWeather } from '@/services/WeatherService'
import { useEffect } from 'react'

function formatDateTime(date: Date): string {
	const pad = (n: number) => n.toString().padStart(2, '0')
	const year = date.getFullYear()
	const month = pad(date.getMonth() + 1)
	const day = pad(date.getDate())
	const hours = pad(date.getHours())
	const minutes = pad(date.getMinutes())
	const seconds = pad(date.getSeconds())

	return `${year}-${month}-${day}T${hours}:${minutes}:${seconds}`
}

export function useInfoSync() {
	useEffect(() => {
		const initAutoConnect = async () => {
			const autoConnect = await StorageService.getAutoConnect()
			const lastId = await StorageService.getLastDeviceId()

			if (autoConnect && lastId && bleService.connectionState === 'idle') {
				try {
					await bleService.connect(lastId)
				} catch (e) {}
			}
		}

		initAutoConnect()

		const unsubscribe = bleService.onEvent(async event => {
			if (event.type === 'connectionStateChange') {
				if (event.state === 'connected') {
					try {
						const nowStr = formatDateTime(new Date())
						await bleService.send(`TIME:${nowStr}`)

						const weatherData = await getWeather('Gdansk')
						if (weatherData) {
							await bleService.send(`WEATHER:${weatherData}`)
						}
					} catch (e) {}
				} else if (event.state === 'disconnected' || event.state === 'failed') {
					const autoConnect = await StorageService.getAutoConnect()
					const lastId = await StorageService.getLastDeviceId()
					if (autoConnect && lastId) {
						setTimeout(() => {
							const state = bleService.connectionState
							if (
								state === 'idle' ||
								state === 'disconnected' ||
								state === 'failed'
							) {
								bleService.connect(lastId).catch(() => {})
							}
						}, 5000)
					}
				}
			} else if (event.type === 'data') {
				console.log(`processing payload: '${event.payload}'`)
				if (event.payload === 'REFRESH_WEATHER') {
					try {
						const weatherData = await getWeather('Gdansk')
						if (weatherData) {
							await bleService.send(`WEATHER:${weatherData}`)
						}
					} catch (e) {}
				} else if (event.payload.startsWith('battery:')) {
					try {
						const val = Number(event.payload.split(':')[1].replace('%', ''))
						if (!isNaN(val)) bleService.setPercentageBattery = val
						console.log(`received battery percentage: ${val}`)
					} catch (e) {}
				}
			}
		})

		return () => {
			unsubscribe()
		}
	}, [])
}
