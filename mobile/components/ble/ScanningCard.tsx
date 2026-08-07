import MaterialIcons from '@expo/vector-icons/MaterialIcons'
import { useEffect, useRef } from 'react'
import { Animated, Easing, Pressable, StyleSheet, View } from 'react-native'

import { ThemedText } from '@/components/themed-text'
import { ThemedView } from '@/components/themed-view'
import { useThemeColor } from '@/hooks/use-theme-color'

export function ScanningCard({ onPress }: { onPress?: () => void }) {
	const primaryColor = useThemeColor({}, 'primary')
	const borderColor = useThemeColor({}, 'border')
	const textSecondary = useThemeColor({}, 'textSecondary')

	const rotate = useRef(new Animated.Value(0)).current
	const rotateReverse = useRef(new Animated.Value(0)).current
	const pulse = useRef(new Animated.Value(1)).current

	useEffect(() => {
		const loop = Animated.loop(
			Animated.timing(rotate, {
				toValue: 1,
				duration: 4000,
				easing: Easing.linear,
				useNativeDriver: true,
			}),
		)
		loop.start()
		return () => loop.stop()
	}, [rotate])

	useEffect(() => {
		const loop = Animated.loop(
			Animated.timing(rotateReverse, {
				toValue: 1,
				duration: 5000,
				easing: Easing.linear,
				useNativeDriver: true,
			}),
		)
		loop.start()
		return () => loop.stop()
	}, [rotateReverse])

	useEffect(() => {
		const loop = Animated.loop(
			Animated.sequence([
				Animated.timing(pulse, {
					toValue: 0.88,
					duration: 1000,
					easing: Easing.inOut(Easing.ease),
					useNativeDriver: true,
				}),
				Animated.timing(pulse, {
					toValue: 1,
					duration: 1000,
					easing: Easing.inOut(Easing.ease),
					useNativeDriver: true,
				}),
			]),
		)
		loop.start()
		return () => loop.stop()
	}, [pulse])

	const spin = rotate.interpolate({
		inputRange: [0, 1],
		outputRange: ['0deg', '360deg'],
	})
	const spinReverse = rotateReverse.interpolate({
		inputRange: [0, 1],
		outputRange: ['360deg', '0deg'],
	})

	return (
		<Pressable
			onPress={onPress}
			style={({ pressed }) => [pressed && { opacity: 0.85 }]}>
			<ThemedView
				color='surfaceContainer'
				style={[styles.glassCard, { borderColor }]}>
				<View style={styles.ringsContainer}>
					<Animated.View
						style={[
							styles.ring,
							styles.ringOuter,
							{ borderColor },
							{ transform: [{ rotate: spin }] },
						]}
					/>
					<Animated.View
						style={[
							styles.ring,
							styles.ringInner,
							{ borderColor },
							{ transform: [{ rotate: spinReverse }] },
						]}
					/>
				</View>

				<View style={styles.scanContent}>
					<Animated.View
						style={[
							styles.scanIconCircle,
							{
								backgroundColor: primaryColor + '15',
								borderColor: primaryColor + '30',
								transform: [{ scale: pulse }],
							},
						]}>
						<MaterialIcons
							name='bluetooth-searching'
							size={32}
							color={primaryColor}
						/>
					</Animated.View>
					<ThemedText style={{ color: textSecondary, fontSize: 15 }}>
						Scanning for devices...
					</ThemedText>
					<ThemedText
						style={{
							color: primaryColor,
							fontSize: 12,
							fontFamily: 'Montserrat_600SemiBold',
							textTransform: 'uppercase',
							letterSpacing: 0.5,
						}}>
						Tap to scan
					</ThemedText>
				</View>
			</ThemedView>
		</Pressable>
	)
}

const CARD_H = 200
const OUTER_RING = 220
const INNER_RING = 160

const styles = StyleSheet.create({
	glassCard: {
		width: '100%',
		height: CARD_H,
		borderRadius: 20,
		borderWidth: 1,
		alignItems: 'center',
		justifyContent: 'center',
		overflow: 'hidden',
		paddingHorizontal: 50,
	},
	ringsContainer: {
		...StyleSheet.absoluteFillObject,
		justifyContent: 'center',
		alignItems: 'center',
	},
	ring: {
		position: 'absolute',
		borderRadius: 9999,
		borderWidth: 1,
		opacity: 0.12,
	},
	ringOuter: {
		width: OUTER_RING,
		height: OUTER_RING,
	},
	ringInner: {
		width: INNER_RING,
		height: INNER_RING,
	},
	scanContent: {
		alignItems: 'center',
		gap: 14,
		zIndex: 1,
	},
	scanIconCircle: {
		width: 64,
		height: 64,
		borderRadius: 32,
		borderWidth: 1,
		alignItems: 'center',
		justifyContent: 'center',
	},
})
