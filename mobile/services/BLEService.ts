import { Buffer } from 'buffer'
import { PermissionsAndroid, Platform } from 'react-native'
import { BleManager, type Characteristic } from 'react-native-ble-plx'

export const NUS = {
	service: '6e7bdab4-e3c6-45ce-a0ef-f7e4d2c8ae45',
	rx: '1e5d03d4-a534-4c81-bc9b-3056bf878d15',
	tx: '44d2187d-f65e-4a55-b5fa-00da1726c6f1',
} as const

export type BLEDevice = {
	id: string
	name: string
	rssi: number
}

export type ConnectionState =
	| 'idle'
	| 'scanning'
	| 'connecting'
	| 'connected'
	| 'failed'
	| 'disconnected'

export type BLEServiceEvent =
	| { type: 'deviceDiscovered'; device: BLEDevice }
	| { type: 'scanComplete'; count: number }
	| { type: 'connectionStateChange'; state: ConnectionState }
	| { type: 'data'; payload: string }
	| { type: 'error'; message: string }
	| { type: 'syncUpdate' }

type EventListener = (event: BLEServiceEvent) => void

const SCAN_TIMEOUT_MS = 10_000

class BLEService {
	private _manager: BleManager
	private _connectionState: ConnectionState = 'idle'
	private _connectedDevice: BLEDevice | null = null
	private _listeners = new Set<EventListener>()
	private _discovered = new Map<string, BLEDevice>()
	private _scanTimer: ReturnType<typeof setTimeout> | null = null
	private _txSubscription: { remove(): void } | null = null
	private _disconnectSubscription: { remove(): void } | null = null
	private _batteryPercentage: number = NaN
	private _lastTimeSynced: Date | null = null
	constructor() {
		this._manager = new BleManager()
	}

	get connectionState(): ConnectionState {
		return this._connectionState
	}
	get batteryPercentage() {
		return this._batteryPercentage
	}
	public set setPercentageBattery(value: number) {
		this._batteryPercentage = value
		this.emit({ type: 'syncUpdate' })
	}
	get lastTimeSynced() {
		return this._lastTimeSynced
	}
	public set setLastTimeSynced(value: Date) {
		this._lastTimeSynced = value
		this.emit({ type: 'syncUpdate' })
	}
	get connectedDevice(): BLEDevice | null {
		return this._connectedDevice
	}

	onEvent(listener: EventListener): () => void {
		this._listeners.add(listener)
		return () => {
			this._listeners.delete(listener)
		}
	}

	private emit(event: BLEServiceEvent): void {
		for (const fn of this._listeners) {
			try {
				fn(event)
			} catch {}
		}
	}

	private setState(state: ConnectionState): void {
		this._connectionState = state
		this.emit({ type: 'connectionStateChange', state })
	}

	async requestPermissions(): Promise<boolean> {
		if (Platform.OS === 'ios') {
			return true
		}

		if (Platform.OS !== 'android') return false

		const apiLevel = parseInt(Platform.Version.toString(), 10)

		if (apiLevel < 31) {
			const granted = await PermissionsAndroid.request(
				PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
			)
			return granted === PermissionsAndroid.RESULTS.GRANTED
		}

		const result = await PermissionsAndroid.requestMultiple([
			PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
			PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
			PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
		])

		return (
			result['android.permission.BLUETOOTH_CONNECT'] ===
				PermissionsAndroid.RESULTS.GRANTED &&
			result['android.permission.BLUETOOTH_SCAN'] ===
				PermissionsAndroid.RESULTS.GRANTED &&
			result['android.permission.ACCESS_FINE_LOCATION'] ===
				PermissionsAndroid.RESULTS.GRANTED
		)
	}

	startScan(): void {
		if (this._connectionState === 'scanning') return

		this._clearScan()
		this._discovered.clear()
		this.setState('scanning')

		this._scanTimer = setTimeout(() => {
			this.stopScan()
			this.emit({ type: 'scanComplete', count: this._discovered.size })
		}, SCAN_TIMEOUT_MS)

		this._manager
			.connectedDevices([NUS.service])
			.then(devices => {
				for (const dev of devices) {
					const name = dev.localName ?? dev.name ?? ''
					if (name === 'Round-Display') {
						const device: BLEDevice = {
							id: dev.id,
							name,
							rssi: dev.rssi ?? -50,
						}
						this._discovered.set(device.id, device)
						this.emit({ type: 'deviceDiscovered', device })
					}
				}
			})
			.catch(() => {})

		this._manager
			.startDeviceScan(
				[NUS.service],
				{ allowDuplicates: false },
				(error, scannedDevice) => {
					if (error) {
						this.emit({
							type: 'error',
							message: `Scan error: ${error.message}`,
						})
						return
					}
					if (!scannedDevice) return

					const name = scannedDevice.localName ?? scannedDevice.name ?? ''
					const rssi = scannedDevice.rssi ?? -100

					if (name !== 'Round-Display') return

					const device: BLEDevice = {
						id: scannedDevice.id,
						name,
						rssi,
					}

					this._discovered.set(device.id, device)
					this.emit({ type: 'deviceDiscovered', device })
				},
			)
			.catch((err: Error) => {
				this.emit({
					type: 'error',
					message: `Failed to start scan: ${err.message}`,
				})
				this.setState('idle')
			})
	}

	stopScan(): void {
		this._clearScan()
		this._manager.stopDeviceScan().catch(() => {})
		if (this._connectionState === 'scanning') {
			this.setState('idle')
		}
	}

	private _clearScan(): void {
		if (this._scanTimer) {
			clearTimeout(this._scanTimer)
			this._scanTimer = null
		}
	}

	async connect(deviceId: string): Promise<void> {
		if (
			this._connectionState === 'connecting' ||
			this._connectionState === 'connected'
		) {
			console.warn('BLEService: Already connecting or connected')
			return
		}

		this.setState('connecting')

		try {
			const device = await this._manager.connectToDevice(deviceId, {
				autoConnect: false,
				timeout: 8000,
			})

			if (Platform.OS === 'android') {
				try {
					await device.requestMTU(512)
				} catch (e) {}
			}

			await device.discoverAllServicesAndCharacteristics()

			const services = await device.services()
			let rxChar: Characteristic | null = null
			let txChar: Characteristic | null = null

			for (const svc of services) {
				if (svc.uuid.toLowerCase() !== NUS.service.toLowerCase()) continue
				const chars = await svc.characteristics()
				for (const ch of chars) {
					const uuid = ch.uuid.toLowerCase()
					if (uuid === NUS.rx.toLowerCase()) rxChar = ch
					if (uuid === NUS.tx.toLowerCase()) txChar = ch
				}
			}

			if (!rxChar || !txChar) {
				throw new Error(
					'NUS service found but missing required characteristics',
				)
			}

			this._txSubscription = txChar.monitor((error, characteristic) => {
				if (error) {
					this.emit({
						type: 'error',
						message: `Notify error: ${error.message}`,
					})
					return
				}
				const value = characteristic?.value
				if (value) {
					const decoded = Buffer.from(value, 'base64').toString('utf-8')
					this.emit({ type: 'data', payload: decoded })
				}
			})

			this._connectedDevice = {
				id: device.id,
				name: device.localName ?? device.name ?? 'Round-Display',
				rssi: device.rssi ?? -100,
			}

			this.setState('connected')
			
			this._disconnectSubscription = this._manager.onDeviceDisconnected(
				device.id,
				(error, disconnectedDevice) => {
					this._connectedDevice = null
					this.setState('disconnected')
					if (this._disconnectSubscription) {
						this._disconnectSubscription.remove()
						this._disconnectSubscription = null
					}
				}
			)
		} catch (err) {
			this.setState('failed')
			const message = err instanceof Error ? err.message : 'Connection failed'
			this.emit({ type: 'error', message })
			throw err
		}
	}

	async disconnect(): Promise<void> {
		try {
			if (this._txSubscription) {
				this._txSubscription.remove()
				this._txSubscription = null
			}
			if (this._disconnectSubscription) {
				this._disconnectSubscription.remove()
				this._disconnectSubscription = null
			}
			if (this._connectedDevice) {
				await this._manager.cancelDeviceConnection(this._connectedDevice.id)
			}
		} catch {}
		this._connectedDevice = null
		this.setState('disconnected')
	}

	private _sendQueue: Promise<void> = Promise.resolve()

	async send(data: string): Promise<void> {
		if (this._connectionState !== 'connected' || !this._connectedDevice) {
			console.warn('BLEService: Cannot send data, device is not connected')
			return
		}

		const base64 = Buffer.from(data, 'utf-8').toString('base64')
		const deviceId = this._connectedDevice.id

		const task = this._sendQueue.then(async () => {
			if (this._connectionState !== 'connected') return
			await this._manager.writeCharacteristicWithResponseForDevice(
				deviceId,
				NUS.service,
				NUS.rx,
				base64,
			)
		}).catch(err => {
			console.warn('BLEService: Failed to send queued data', err)
		})

		this._sendQueue = task.then(() => {})
		await task
	}

	destroy(): void {
		this._clearScan()
		if (this._txSubscription) {
			this._txSubscription.remove()
			this._txSubscription = null
		}
		if (this._disconnectSubscription) {
			this._disconnectSubscription.remove()
			this._disconnectSubscription = null
		}
		this._listeners.clear()
		this._manager.destroy()
	}
}

export const bleService = new BLEService()
