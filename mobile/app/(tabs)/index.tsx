import MaterialIcons from '@expo/vector-icons/MaterialIcons'
import { useEffect, useRef, useState } from 'react'
import { Animated, Pressable, StyleSheet, Text } from 'react-native'
import AnimatedReanimated, { FadeIn, FadeInUp } from 'react-native-reanimated'
import Svg, { Circle } from 'react-native-svg'

import { ThemedText } from '@/components/themed-text'
import { ThemedView } from '@/components/themed-view'
import { typography } from '@/constants/theme'
import { useThemeColor } from '@/hooks/use-theme-color'
import { bleService } from '@/services/BLEService'

const CIRCUMFERENCE = 2 * Math.PI * 45
const AnimatedCircle = Animated.createAnimatedComponent(Circle)
function formatTime(date: Date | null): string {
	if (date == null) {
		return '--:--'
	}
	const pad = (n: number) => n.toString().padStart(2, '0')
	const hours = pad(date.getHours())
	const minutes = pad(date.getMinutes())

	return `${hours}:${minutes}`
}
function PulsingDot({ color }: { color: string }) {
	const opacity = useRef(new Animated.Value(1)).current

	useEffect(() => {
		const loop = Animated.loop(
			Animated.sequence([
				Animated.timing(opacity, {
					toValue: 0.3,
					duration: 800,
					useNativeDriver: true,
				}),
				Animated.timing(opacity, {
					toValue: 1,
					duration: 800,
					useNativeDriver: true,
				}),
			]),
		)
		loop.start()
		return () => loop.stop()
	}, [])

	return (
		<Animated.View style={{ opacity }}>
			<MaterialIcons name='circle' size={10} color={color} />
		</Animated.View>
	)
}

function SettingsRow({
	icon,
	label,
	children,
	delay = 0,
}: {
	icon: keyof typeof MaterialIcons.glyphMap
	label: string
	children: React.ReactNode
	delay?: number
}) {
	const borderColor = useThemeColor({}, 'border')
	const iconColor = useThemeColor({}, 'icon')
	const textColor = useThemeColor({}, 'icon')

	return (
		<AnimatedReanimated.View
			entering={FadeInUp.duration(400).delay(delay).springify()}
			style={{ width: '100%', alignItems: 'center' }}>
			<ThemedView
				color='surfaceContainer'
				style={[styles.card, { borderColor }]}>
				<MaterialIcons name={icon} size={20} color={iconColor} />
				<ThemedText
					style={[
						{
							color: textColor,
							textTransform: 'uppercase',
							fontSize: 14,
							fontFamily: typography.label.fontFamily,
						},
						styles.cardLabel,
					]}>
					{label}
				</ThemedText>
				<ThemedView style={styles.cardRight}>{children}</ThemedView>
			</ThemedView>
		</AnimatedReanimated.View>
	)
}

export default function HomeScreen() {
	const trackColor = useThemeColor({}, 'border')
	const primaryColor = useThemeColor({}, 'primary')
	const iconColor = useThemeColor({}, 'icon')
	const errorColor = useThemeColor({}, 'error')

	const [brightness, setBrightness] = useState(60)
	const [isConnected, setIsConnected] = useState(
		bleService.connectionState === 'connected',
	)
	const [batteryPercentage, setBatteryPercentage] = useState(
		bleService.batteryPercentage,
	)
	const [lastSynced, setLastSynced] = useState(bleService.lastTimeSynced)

	const targetOffset =
		CIRCUMFERENCE *
		(1 - (isNaN(batteryPercentage) ? 0 : batteryPercentage) / 100)

	const animOffset = useRef(new Animated.Value(CIRCUMFERENCE)).current

	useEffect(() => {
		Animated.timing(animOffset, {
			toValue: targetOffset,
			duration: 1400,
			useNativeDriver: true,
		}).start()
	}, [targetOffset])
	useEffect(() => {
		const timer = setTimeout(() => {
			console.log(`sending brightness: ${brightness}`)
			bleService.send(`brightness:${brightness}`).catch(() => {})
		}, 400)
		return () => clearTimeout(timer)
	}, [brightness])
	useEffect(() => {
		const unsub = bleService.onEvent(async event => {
			if (event.type === 'connectionStateChange') {
				setIsConnected(event.state === 'connected')
				setLastSynced(bleService.lastTimeSynced)
			}
			if (
				event.type === 'connectionStateChange' ||
				('payload' in event &&
					typeof event.payload === 'string' &&
					event.payload.startsWith('battery:'))
			) {
				setBatteryPercentage(bleService.batteryPercentage)
				setLastSynced(bleService.lastTimeSynced)
			}
		})
		return unsub
	}, [])

	return (
		<ThemedView style={styles.root}>
			<ThemedView style={styles.topArea}>
				<AnimatedReanimated.View
					entering={FadeIn.duration(600).springify()}
					style={{ alignItems: 'center' }}>
					<ThemedView
						style={[styles.statusWrapper, { borderColor: trackColor }]}>
						<Svg
							viewBox='0 0 100 100'
							width={210}
							height={210}
							style={styles.progressSvg}>
							<Circle
								cx='50'
								cy='50'
								r='45'
								fill='transparent'
								stroke={trackColor}
								strokeWidth='2'
							/>
							<AnimatedCircle
								cx='50'
								cy='50'
								r='45'
								fill='transparent'
								stroke={primaryColor}
								strokeWidth='2'
								strokeLinecap='round'
								strokeDasharray={CIRCUMFERENCE}
								strokeDashoffset={animOffset}
							/>
						</Svg>

						<MaterialIcons name='watch' size={28} color={iconColor} />

						<ThemedText
							style={{
								color: primaryColor,
								fontFamily: typography.display.fontFamily,
								fontSize: typography.display.fontSize,
								lineHeight: typography.display.lineHeight,
								letterSpacing: -4,
							}}>
							{isNaN(batteryPercentage) ? '--' : batteryPercentage}
							<Text style={{ fontSize: 24, letterSpacing: 0 }}>%</Text>
						</ThemedText>

						<ThemedView
							lightColor={
								isConnected
									? 'rgba(212, 232, 212, 0.5)'
									: 'rgba(232, 212, 212, 0.5)'
							}
							darkColor={
								isConnected ? 'rgba(27, 43, 30, 0.5)' : 'rgba(43, 27, 27, 0.5)'
							}
							style={[styles.connectedStatus, { borderColor: trackColor }]}>
							<PulsingDot color={isConnected ? primaryColor : errorColor} />
							<ThemedText
								style={{
									color: isConnected ? primaryColor : errorColor,
									textTransform: 'uppercase',
									fontSize: typography.label.fontSize,
									fontFamily: typography.label.fontFamily,
								}}>
								{isConnected ? 'connected' : 'disconnected'}
							</ThemedText>
						</ThemedView>
					</ThemedView>
				</AnimatedReanimated.View>
			</ThemedView>

			<ThemedView style={styles.bottomCards}>
				<SettingsRow icon='brightness-6' label='Brightness' delay={200}>
					<Pressable
						onPress={() => setBrightness(v => Math.max(0, v - 10))}
						style={styles.brightBtn}>
						<MaterialIcons name='remove' size={18} color={iconColor} />
					</Pressable>
					<ThemedText
						type='defaultSemiBold'
						style={{ color: primaryColor, minWidth: 36, textAlign: 'center' }}>
						{brightness}%
					</ThemedText>
					<Pressable
						onPress={() => setBrightness(v => Math.min(100, v + 10))}
						style={styles.brightBtn}>
						<MaterialIcons name='add' size={18} color={iconColor} />
					</Pressable>
				</SettingsRow>

				<SettingsRow icon='schedule' label='Last synced' delay={350}>
					<ThemedText type='default' style={{ color: iconColor }}>
						{formatTime(lastSynced)}
					</ThemedText>
				</SettingsRow>

				<SettingsRow icon='settings' label='Firmware' delay={500}>
					<ThemedText type='default' style={{ color: iconColor }}>
						v1.0.3
					</ThemedText>
				</SettingsRow>
			</ThemedView>
		</ThemedView>
	)
}

const styles = StyleSheet.create({
	root: {
		flex: 1,
	},
	topArea: {
		flex: 1,
		alignItems: 'center',
		justifyContent: 'center',
	},
	statusWrapper: {
		width: 210,
		height: 210,
		borderRadius: 105,
		borderWidth: 1,
		alignItems: 'center',
		justifyContent: 'center',
		gap: 8,
	},
	progressSvg: {
		position: 'absolute',
		top: 0,
		left: 0,
		transform: [{ rotate: '-90deg' }],
	},
	connectedStatus: {
		flexDirection: 'row',
		alignItems: 'center',
		justifyContent: 'center',
		gap: 4,
		width: 110,
		height: 25,
		borderRadius: 20,
		borderWidth: 1,
		paddingHorizontal: 10,
	},
	bottomCards: {
		alignItems: 'center',
		gap: 10,
		paddingBottom: 20,
	},
	card: {
		flexDirection: 'row',
		alignItems: 'center',
		gap: 10,
		width: '85%',
		paddingVertical: 14,
		paddingHorizontal: 16,
		borderRadius: 16,
		borderWidth: 1,
	},
	cardLabel: {
		flex: 1,
	},
	cardRight: {
		flexDirection: 'row',
		alignItems: 'center',
		gap: 6,
		backgroundColor: 'transparent',
	},
	brightBtn: {
		padding: 4,
	},
})
