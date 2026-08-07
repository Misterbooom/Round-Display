import MaterialIcons from '@expo/vector-icons/MaterialIcons'
import { StyleSheet, Switch } from 'react-native'
import AnimatedReanimated, { FadeInUp } from 'react-native-reanimated'

import { ThemedText } from '@/components/themed-text'
import { ThemedView } from '@/components/themed-view'
import { useThemeColor } from '@/hooks/use-theme-color'

export function ToggleRow({
	icon,
	label,
	value,
	onValueChange,
	delay = 0,
	focusKey = 0,
}: {
	icon: keyof typeof MaterialIcons.glyphMap
	label: string
	value: boolean
	onValueChange: (v: boolean) => void
	delay?: number
	focusKey?: number
}) {
	const borderColor = useThemeColor({}, 'border')
	const iconColor = useThemeColor({}, 'icon')
	const primaryColor = useThemeColor({}, 'primary')
	const textColor = useThemeColor({}, 'icon')

	return (
		<AnimatedReanimated.View
			key={focusKey}
			entering={FadeInUp.duration(400).delay(delay).springify()}
			style={{ width: '100%', alignItems: 'center' }}>
			<ThemedView
				color='surfaceContainer'
				style={[styles.toggleCard, { borderColor }]}>
				<MaterialIcons name={icon} size={20} color={iconColor} />
				<ThemedText style={[styles.toggleLabel, { color: textColor }]}>
					{label}
				</ThemedText>
				<Switch
					value={value}
					onValueChange={onValueChange}
					trackColor={{ false: borderColor, true: primaryColor + '60' }}
					thumbColor={value ? primaryColor : iconColor}
					ios_backgroundColor={borderColor}
				/>
			</ThemedView>
		</AnimatedReanimated.View>
	)
}

const styles = StyleSheet.create({
	toggleCard: {
		flexDirection: 'row',
		alignItems: 'center',
		gap: 10,
		width: '100%',
		paddingVertical: 14,
		paddingHorizontal: 16,
		borderRadius: 16,
		borderWidth: 1,
	},
	toggleLabel: {
		flex: 1,
		fontSize: 14,
		fontFamily: 'Montserrat_600SemiBold',
		textTransform: 'uppercase',
		letterSpacing: 0.4,
	},
})
