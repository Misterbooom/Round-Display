import MaterialIcons from '@expo/vector-icons/MaterialIcons'
import React, { useState } from 'react'
import { Alert, Pressable, ScrollView, StyleSheet } from 'react-native'
import { BleAPI } from '@/services/bleService'
import { ThemedText } from '@/components/themed-text'
import { ThemedView } from '@/components/themed-view'
import { useThemeColor } from '@/hooks/use-theme-color'
import { typography } from '@/constants/theme'

function ActionButton({ icon, label, onPress }: { icon: keyof typeof MaterialIcons.glyphMap, label: string, onPress: () => void }) {
	const borderColor = useThemeColor({}, 'border')
	const iconColor = useThemeColor({}, 'icon')
	const primaryColor = useThemeColor({}, 'primary')

	return (
		<Pressable onPress={onPress}>
			<ThemedView color="surfaceContainer" style={[styles.card, { borderColor }]}>
				<MaterialIcons name={icon} size={24} color={iconColor} />
				<ThemedText style={[{ color: primaryColor, fontFamily: typography.label.fontFamily }, styles.cardLabel]}>
					{label}
				</ThemedText>
			</ThemedView>
		</Pressable>
	)
}

export default function DeveloperScreen() {
	const [status, setStatus] = useState<string>('Unknown')

	const checkConnection = async () => {
		try {
			const state = await BleAPI.getConnectionState()
			setStatus(state)
			Alert.alert("Connection State", state)
		} catch (e: any) {
			Alert.alert("Error", e.message)
		}
	}

	const sendPing = async () => {
		try {
			await BleAPI.sendPing()
			Alert.alert("Success", "Ping sent")
		} catch (e: any) {
			Alert.alert("Error", e.message)
		}
	}

	const disconnect = async () => {
		try {
			await BleAPI.disconnect()
			Alert.alert("Success", "Disconnected")
		} catch (e: any) {
			Alert.alert("Error", e.message)
		}
	}

	const sendTestTime = async () => {
		try {
			await BleAPI.sendTime(new Date().toISOString())
			Alert.alert("Success", "Time sent")
		} catch (e: any) {
			Alert.alert("Error", e.message)
		}
	}

	return (
		<ScrollView style={styles.root} contentContainerStyle={styles.content}>
			<ThemedText type="title" style={styles.title}>Developer Tools</ThemedText>
			
			<ActionButton icon="network-check" label={`Check State (Current: ${status})`} onPress={checkConnection} />
			<ActionButton icon="sensors" label="Send Ping" onPress={sendPing} />
			<ActionButton icon="access-time" label="Send Current Time" onPress={sendTestTime} />
			<ActionButton icon="bluetooth-disabled" label="Force Disconnect" onPress={disconnect} />
		</ScrollView>
	)
}

const styles = StyleSheet.create({
	root: {
		flex: 1,
	},
	content: {
		padding: 20,
		gap: 16,
	},
	title: {
		marginBottom: 10,
	},
	card: {
		flexDirection: 'row',
		alignItems: 'center',
		gap: 16,
		padding: 16,
		borderRadius: 16,
		borderWidth: 1,
	},
	cardLabel: {
		fontSize: 16,
		textTransform: 'uppercase',
	},
})
