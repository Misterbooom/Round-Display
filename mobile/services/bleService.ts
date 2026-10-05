import {
	Alert,
	EmitterSubscription,
	NativeEventEmitter,
	NativeModules,
	PermissionsAndroid,
	Platform,
} from 'react-native'

export type BleConnectionStatus =
	| 'disconnected'
	| 'scanning'
	| 'connecting'
	| 'connected'
	| 'reconnecting'

export type BleSnapshot = {
	status: BleConnectionStatus
	batteryLevel: number | null
	weatherRequestPending: boolean
	sessionActive: boolean
	serviceRunning: boolean
}

export type BleErrorDetails = {
	code?: string
	message: string
}

type NativeBleModule = {
	startScanAndConnect(): Promise<boolean>
	autoConnect(): Promise<boolean>
	disconnect(): Promise<boolean>
	sendTime(time: string): Promise<boolean>
	sendWeather(weather: string): Promise<boolean>
	sendBrightness(brightness: number): Promise<boolean>
	sendPing(): Promise<boolean>
	getConnectionState(): Promise<BleConnectionStatus>
	getSnapshot(): Promise<BleSnapshot>
	startService(): Promise<boolean>
	stopService(): Promise<boolean>
	isServiceRunning(): Promise<boolean>
	isIgnoringBatteryOptimizations(): Promise<boolean>
	requestIgnoreBatteryOptimizations(): Promise<boolean>
	setCity(city: string): Promise<boolean>
	getCity(): Promise<string>
	addListener(eventName: string): void
	removeListeners(count: number): void
}

type ConnectionStateEvent = { status: BleConnectionStatus }
type BatteryUpdateEvent = { batteryLevel: number }
type CustomNotificationEvent = { raw: string }

export type BleEventHandlers = {
	onConnectionChange?: (status: BleConnectionStatus) => void
	onBatteryUpdate?: (batteryLevel: number) => void
	onWeatherRequested?: () => void
	onPongReceived?: () => void
	onCustomNotification?: (message: string) => void
	onKotlinLog?: (message: string) => void
}

const nativeBleModule = NativeModules.BleService as NativeBleModule | undefined

const bleEventEmitter = nativeBleModule
	? new NativeEventEmitter(nativeBleModule)
	: null

if (bleEventEmitter) {
	bleEventEmitter.addListener('onKotlinLog', (message: string) => {
		console.log('Native BLE Log:', message)
	})
}

function unavailableError(): Error {
	const error = new Error('Bluetooth is unavailable in this build.')
	Object.assign(error, { code: 'BLE_UNAVAILABLE' })
	return error
}

function getNativeBleModule(): NativeBleModule {
	if (!nativeBleModule) throw unavailableError()
	return nativeBleModule
}

function rejectInvalidArgument(message: string): Promise<never> {
	return Promise.reject(new Error(message))
}

export function getBleErrorDetails(error: unknown): BleErrorDetails {
	if (error instanceof Error) {
		const code =
			typeof (error as Error & { code?: unknown }).code === 'string'
				? (error as Error & { code: string }).code
				: undefined
		return { code, message: error.message }
	}

	if (typeof error === 'object' && error != null) {
		const value = error as { code?: unknown; message?: unknown }
		return {
			code: typeof value.code === 'string' ? value.code : undefined,
			message:
				typeof value.message === 'string'
					? value.message
					: 'Bluetooth request failed.',
		}
	}

	return { message: typeof error === 'string' ? error : 'Bluetooth request failed.' }
}

export type PermissionCheckResult = {
	bluetoothGranted: boolean
	notificationsGranted: boolean
}

/**
 * Request all permissions required for Android 12+ (API 31+) & Android 13+ (API 33+).
 * Bluetooth scan & connect permissions are mandatory. Notification permission is recommended
 * so the ongoing foreground service status notification is visible to the user.
 */
export async function ensureBlePermissions(): Promise<boolean> {
	if (Platform.OS !== 'android') return true

	const apiLevel = Number(Platform.Version)

	if (apiLevel >= 31) {
		const permissionsToRequest = [
			PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
			PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
		]

		if (apiLevel >= 33) {
			permissionsToRequest.push(PermissionsAndroid.PERMISSIONS.POST_NOTIFICATIONS)
		}

		const statuses = await PermissionsAndroid.requestMultiple(permissionsToRequest)

		const bluetoothGranted =
			statuses[PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN] ===
				PermissionsAndroid.RESULTS.GRANTED &&
			statuses[PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT] ===
				PermissionsAndroid.RESULTS.GRANTED

		return bluetoothGranted
	}

	const status = await PermissionsAndroid.request(
		PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
	)
	return status === PermissionsAndroid.RESULTS.GRANTED
}

export const ensurePermissions = ensureBlePermissions

export const BleAPI = {
	tryToConnect: (
		timeoutMs: number = 15_000,
		showAlert: boolean = true,
	): Promise<boolean> => {
		return new Promise<boolean>((resolve, reject) => {
			let isSettled = false

			const timer = setTimeout(async () => {
				if (isSettled) return
				isSettled = true

				try {
					await getNativeBleModule().disconnect()
				} catch {
					// Ignore disconnect failure on timeout cleanup
				}

				if (showAlert) {
					Alert.alert(
						'Connection Timeout',
						'Round Display has not connected after 15 seconds. Please make sure the device is turned on and in range.',
					)
				}

				const timeoutError = new Error('Device has not connected after 15 seconds.')
				Object.assign(timeoutError, { code: 'CONNECTION_TIMEOUT' })
				reject(timeoutError)
			}, timeoutMs)

			getNativeBleModule()
				.startScanAndConnect()
				.then(result => {
					if (isSettled) return
					isSettled = true
					clearTimeout(timer)
					resolve(result)
				})
				.catch(error => {
					if (isSettled) return
					isSettled = true
					clearTimeout(timer)
					reject(error)
				})
		})
	},
	connect: (): Promise<boolean> => getNativeBleModule().startScanAndConnect(),
	autoConnect: (): Promise<boolean> => getNativeBleModule().autoConnect(),
	disconnect: (): Promise<boolean> => getNativeBleModule().disconnect(),
	getConnectionState: (): Promise<BleConnectionStatus> =>
		getNativeBleModule().getConnectionState(),
	getSnapshot: (): Promise<BleSnapshot> => getNativeBleModule().getSnapshot(),

	startService: (): Promise<boolean> => getNativeBleModule().startService(),
	stopService: (): Promise<boolean> => getNativeBleModule().stopService(),
	isServiceRunning: (): Promise<boolean> => getNativeBleModule().isServiceRunning(),

	isIgnoringBatteryOptimizations: (): Promise<boolean> =>
		getNativeBleModule().isIgnoringBatteryOptimizations(),
	requestIgnoreBatteryOptimizations: (): Promise<boolean> =>
		getNativeBleModule().requestIgnoreBatteryOptimizations(),

	sendTime: (time: string): Promise<boolean> => {
		if (!time.trim()) return rejectInvalidArgument('Time cannot be empty.')
		return getNativeBleModule().sendTime(time)
	},
	sendWeather: (weather: string): Promise<boolean> => {
		if (!weather.trim()) return rejectInvalidArgument('Weather payload cannot be empty.')
		return getNativeBleModule().sendWeather(weather)
	},
	sendBrightness: (brightness: number): Promise<boolean> => {
		if (!Number.isInteger(brightness) || brightness < 0 || brightness > 100) {
			return rejectInvalidArgument('Brightness must be an integer between 0 and 100.')
		}
		// Firmware brightness is 0-255; UI uses 0-100%
		const scaled = Math.round((brightness / 100) * 255)
		return getNativeBleModule().sendBrightness(scaled)
	},
	sendPing: (): Promise<boolean> => getNativeBleModule().sendPing(),
	setCity: (city: string): Promise<boolean> => {
		if (!city.trim()) return rejectInvalidArgument('City cannot be empty.')
		return getNativeBleModule().setCity(city.trim())
	},
	getCity: (): Promise<string> => getNativeBleModule().getCity(),
}

export function subscribeToBleEvents(handlers: BleEventHandlers): () => void {
	if (!bleEventEmitter) return () => undefined

	const subscriptions: EmitterSubscription[] = []

	if (handlers.onConnectionChange) {
		subscriptions.push(
			bleEventEmitter.addListener(
				'onConnectionStateChange',
				(event: ConnectionStateEvent) => handlers.onConnectionChange?.(event.status),
			),
		)
	}
	if (handlers.onBatteryUpdate) {
		subscriptions.push(
			bleEventEmitter.addListener('onBatteryUpdate', (event: BatteryUpdateEvent) =>
				handlers.onBatteryUpdate?.(event.batteryLevel),
			),
		)
	}
	if (handlers.onWeatherRequested) {
		subscriptions.push(
			bleEventEmitter.addListener('onWeatherRequested', handlers.onWeatherRequested),
		)
	}
	if (handlers.onPongReceived) {
		subscriptions.push(
			bleEventEmitter.addListener('onPongReceived', handlers.onPongReceived),
		)
	}
	if (handlers.onCustomNotification) {
		subscriptions.push(
			bleEventEmitter.addListener(
				'onCustomNotification',
				(event: CustomNotificationEvent) =>
					handlers.onCustomNotification?.(event.raw),
			),
		)
	}
	if (handlers.onKotlinLog) {
		subscriptions.push(
			bleEventEmitter.addListener('onKotlinLog', (message: string) =>
				handlers.onKotlinLog?.(message),
			),
		)
	}

	return () => subscriptions.forEach(subscription => subscription.remove())
}

// ------------------------------------------------------------------------
// Compatibility wrappers (migrated from legacy react-native-background-actions)
// ------------------------------------------------------------------------

export async function startBackgroundSync(): Promise<void> {
	await BleAPI.startService()
}

export async function stopBackgroundSync(): Promise<void> {
	await BleAPI.stopService()
}

export function isBackgroundSyncRunning(): Promise<boolean> {
	return BleAPI.isServiceRunning()
}
