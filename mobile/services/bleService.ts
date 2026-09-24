import {
	Alert,
	EmitterSubscription,
	NativeEventEmitter,
	NativeModules,
	PermissionsAndroid,
	Platform,
} from 'react-native'
import BackgroundService from 'react-native-background-actions'

export type BleConnectionStatus =
	| 'disconnected'
	| 'scanning'
	| 'connecting'
	| 'connected'

export type BleErrorDetails = {
	code?: string
	message: string
}

type NativeBleModule = {
	startScanAndConnect(): Promise<boolean>
	disconnect(): Promise<boolean>
	sendTime(time: string): Promise<boolean>
	sendWeather(weather: string): Promise<boolean>
	sendBrightness(brightness: number): Promise<boolean>
	sendPing(): Promise<boolean>
	getConnectionState(): Promise<BleConnectionStatus>
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
}

const nativeBleModule = NativeModules.BleService as NativeBleModule | undefined

const bleEventEmitter = nativeBleModule
	? new NativeEventEmitter(nativeBleModule)
	: null
bleEventEmitter.addListener('onKotlinLog', (message) => {
	console.log('Native Log:', message);
});
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
		const code = typeof (error as Error & { code?: unknown }).code === 'string'
			? (error as Error & { code: string }).code
			: undefined
		return { code, message: error.message }
	}

	if (typeof error === 'object' && error != null) {
		const value = error as { code?: unknown; message?: unknown }
		return {
			code: typeof value.code === 'string' ? value.code : undefined,
			message: typeof value.message === 'string' ? value.message : 'Bluetooth request failed.',
		}
	}

	return { message: typeof error === 'string' ? error : 'Bluetooth request failed.' }
}

export async function ensureBlePermissions(): Promise<boolean> {
	if (Platform.OS !== 'android') return true

	if (Number(Platform.Version) >= 31) {
		const statuses = await PermissionsAndroid.requestMultiple([
			PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
			PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
		])

		return (
			statuses[PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN] ===
				PermissionsAndroid.RESULTS.GRANTED &&
			statuses[PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT] ===
				PermissionsAndroid.RESULTS.GRANTED
		)
	}

	const status = await PermissionsAndroid.request(
		PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
	)
	return status === PermissionsAndroid.RESULTS.GRANTED
}

export const ensurePermissions = ensureBlePermissions


export const BleAPI = {
	startScanAndConnect: (timeoutMs: number = 15_000, showAlert: boolean = true): Promise<boolean> => {
		return new Promise<boolean>((resolve, reject) => {
			let isSettled = false

			const timer = setTimeout(async () => {
				if (isSettled) return
				isSettled = true

				try {
					await getNativeBleModule().disconnect()
				} catch {
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
	disconnect: (): Promise<boolean> => getNativeBleModule().disconnect(),
	getConnectionState: (): Promise<BleConnectionStatus> =>
		getNativeBleModule().getConnectionState(),
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
		return getNativeBleModule().sendBrightness(brightness)
	},
	sendPing: (): Promise<boolean> => getNativeBleModule().sendPing(),
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
				(event: CustomNotificationEvent) => handlers.onCustomNotification?.(event.raw),
			),
		)
	}

	return () => subscriptions.forEach(subscription => subscription.remove())
}

const sleep = (time: number) => new Promise<void>(resolve => setTimeout(resolve, time))

type BackgroundTaskParameters = { delay: number }

const backgroundSyncTask = async ({ delay }: BackgroundTaskParameters) => {
	while (BackgroundService.isRunning()) {
		await sleep(delay)
	}
}

const backgroundOptions = {
	taskName: 'RoundDisplaySync',
	taskTitle: 'Round Display Connected',
	taskDesc: 'Keeping the Round Display connection available',
	taskIcon: {
		name: 'ic_launcher',
		type: 'mipmap',
	},
	color: '#000000',
	parameters: {
		delay: 60_000,
	},
}

export async function startBackgroundSync(): Promise<void> {
	if (!BackgroundService.isRunning()) {
		await BackgroundService.start(backgroundSyncTask, backgroundOptions)
	}
}

export async function stopBackgroundSync(): Promise<void> {
	if (BackgroundService.isRunning()) {
		await BackgroundService.stop()
	}
}

export function isBackgroundSyncRunning(): boolean {
	return BackgroundService.isRunning()
}
