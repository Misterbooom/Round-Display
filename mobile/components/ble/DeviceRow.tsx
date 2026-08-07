import MaterialIcons from '@expo/vector-icons/MaterialIcons'
import { Pressable, StyleSheet, View } from 'react-native'
import AnimatedReanimated, { FadeInUp } from 'react-native-reanimated'

import { ThemedText } from '@/components/themed-text'
import { ThemedView } from '@/components/themed-view'
import { useThemeColor } from '@/hooks/use-theme-color'

export function DeviceRow({
	icon,
	name,
	status,
	onPress,
	delay = 0,
	focusKey = 0,
}: {
	icon: keyof typeof MaterialIcons.glyphMap
	name: string
	status: string
	onPress?: () => void
	delay?: number
	focusKey?: number
}) {
	const borderColor = useThemeColor({}, 'border')
	const iconColor = useThemeColor({}, 'icon')
	const primaryColor = useThemeColor({}, 'primary')

	return (
		<AnimatedReanimated.View
			key={focusKey}
			entering={FadeInUp.duration(400).delay(delay).springify()}
			style={{ width: '100%', alignItems: 'center' }}>
			<Pressable
				onPress={onPress}
				style={({ pressed }) => [
					styles.deviceCard,
					{ borderColor },
					pressed && { opacity: 0.7 },
				]}>
				<ThemedView
					color='surfaceContainer'
					style={[styles.deviceIconCircle, { borderColor }]}>
					<MaterialIcons name={icon} size={20} color={iconColor} />
				</ThemedView>
				<View style={styles.deviceInfo}>
					<ThemedText type='defaultSemiBold' style={{ fontSize: 15 }}>
						{name}
					</ThemedText>
					<ThemedText
						style={[styles.deviceStatusLabel, { color: primaryColor }]}>
						{status}
					</ThemedText>
				</View>
				<MaterialIcons name='settings' size={20} color={iconColor} />
			</Pressable>
		</AnimatedReanimated.View>
	)
}

const styles = StyleSheet.create({
	deviceCard: {
		flexDirection: 'row',
		alignItems: 'center',
		gap: 12,
		width: '100%',
		paddingVertical: 14,
		paddingHorizontal: 16,
		borderRadius: 16,
		borderWidth: 1,
		backgroundColor: 'transparent',
	},
	deviceIconCircle: {
		width: 40,
		height: 40,
		borderRadius: 20,
		borderWidth: 1,
		alignItems: 'center',
		justifyContent: 'center',
	},
	deviceInfo: {
		flex: 1,
	},
	deviceStatusLabel: {
		fontSize: 12,
		fontFamily: 'Montserrat_600SemiBold',
		textTransform: 'uppercase',
		letterSpacing: 0.5,
		marginTop: 2,
	},
})
