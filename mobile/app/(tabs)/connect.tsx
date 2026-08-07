import { useFocusEffect } from 'expo-router'
import { useCallback, useEffect, useState } from 'react'
import { StyleSheet, View } from 'react-native'
import AnimatedReanimated, { FadeIn } from 'react-native-reanimated'

import { ThemedView } from '@/components/themed-view'
import { type BLEDevice } from '@/services/BLEService'
import { StorageService } from '@/services/StorageService'

import { ConnectModal } from '@/components/ble/ConnectModal'
import { DeviceRow } from '@/components/ble/DeviceRow'
import { ScanningCard } from '@/components/ble/ScanningCard'
import { SectionLabel } from '@/components/ble/SectionLabel'
import { ToggleRow } from '@/components/ble/ToggleRow'
import { bleService } from '@/services/BLEService'
export default function ConnectScreen() {
	const [autoConnect, setAutoConnect] = useState(true)
	const [modalVisible, setModalVisible] = useState(false)
	const [pairedDevice, setPairedDevice] = useState<BLEDevice | null>(bleService.connectedDevice)
	const [focusKey, setFocusKey] = useState(0)
	const [connState, setConnState] = useState(bleService.connectionState)

	useFocusEffect(
		useCallback(() => {
			setFocusKey(prev => prev + 1)
		}, []),
	)

	useEffect(() => {
		StorageService.getAutoConnect().then(setAutoConnect)

		const unsub = bleService.onEvent(event => {
			if (event.type === 'connectionStateChange') {
				setConnState(event.state)
				setPairedDevice(bleService.connectedDevice)
			}
		})
		return unsub
	}, [])

	const handleAutoConnectChange = (val: boolean) => {
		setAutoConnect(val)
		StorageService.setAutoConnect(val)
	}

	const isConnectingOrConnected =
		connState === 'connecting' || connState === 'connected'

	return (
		<ThemedView style={styles.root}>
			{!isConnectingOrConnected && (
				<View style={styles.section}>
					<AnimatedReanimated.View
						key={focusKey}
						entering={FadeIn.duration(600).springify()}
						style={{ alignItems: 'center' }}>
						<ScanningCard onPress={() => setModalVisible(true)} />
					</AnimatedReanimated.View>
				</View>
			)}

			<View style={styles.section}>
				<SectionLabel>Paired Device</SectionLabel>
				<DeviceRow
					focusKey={focusKey}
					icon='watch'
					name={pairedDevice?.name ?? 'No device paired'}
					status={pairedDevice ? 'Connected' : 'Not connected'}
					delay={200}
				/>
			</View>

			<View style={styles.section}>
				<SectionLabel>Settings</SectionLabel>
				<ToggleRow
					focusKey={focusKey}
					icon='bluetooth-connected'
					label='Auto-connect'
					value={autoConnect}
					onValueChange={handleAutoConnectChange}
					delay={350}
				/>
			</View>

			<ConnectModal
				visible={modalVisible}
				onClose={() => setModalVisible(false)}
				onConnected={setPairedDevice}
			/>
		</ThemedView>
	)
}

const styles = StyleSheet.create({
	root: {
		flex: 1,
		paddingHorizontal: 24,
		paddingTop: 50,
	},
	section: {
		marginBottom: 24,
	},
})
