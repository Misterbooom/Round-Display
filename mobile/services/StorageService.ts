import AsyncStorage from '@react-native-async-storage/async-storage'

const AUTO_CONNECT_KEY = '@autoConnect'
const LAST_DEVICE_ID_KEY = '@lastDeviceId'

export const StorageService = {
	async getAutoConnect(): Promise<boolean> {
		try {
			const value = await AsyncStorage.getItem(AUTO_CONNECT_KEY)
			if (value !== null) {
				return value === 'true'
			}
			return true 
		} catch (e) {
			return true
		}
	},

	async setAutoConnect(value: boolean): Promise<void> {
		try {
			await AsyncStorage.setItem(AUTO_CONNECT_KEY, value ? 'true' : 'false')
		} catch (e) {}
	},

	async getLastDeviceId(): Promise<string | null> {
		try {
			return await AsyncStorage.getItem(LAST_DEVICE_ID_KEY)
		} catch (e) {
			return null
		}
	},

	async setLastDeviceId(id: string): Promise<void> {
		try {
			await AsyncStorage.setItem(LAST_DEVICE_ID_KEY, id)
		} catch (e) {}
	},
}
