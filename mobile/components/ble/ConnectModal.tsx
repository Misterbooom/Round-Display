import MaterialIcons from '@expo/vector-icons/MaterialIcons'
import { useCallback, useEffect, useRef, useState } from 'react'
import {
	ActivityIndicator,
	Animated,
	Modal,
	Pressable,
	StyleSheet,
	View,
} from 'react-native'
import AnimatedReanimated, { FadeInUp } from 'react-native-reanimated'

import { ThemedText } from '@/components/themed-text'
import { ThemedView } from '@/components/themed-view'
import { typography } from '@/constants/theme'
import { useThemeColor } from '@/hooks/use-theme-color'
import {
	bleService,
	type BLEDevice,
	type ConnectionState,
} from '@/services/BLEService'
import { StorageService } from '@/services/StorageService'

function rssiBars(rssi: number): number {
	if (rssi > -50) return 4
	if (rssi > -60) return 3
	if (rssi > -70) return 2
	return 1
}

export function ConnectModal({
	visible,
	onClose,
	onConnected,
}: {
	visible: boolean
	onClose: () => void
	onConnected: (device: BLEDevice) => void
}) {
	const borderColor = useThemeColor({}, 'border')
	const primaryColor = useThemeColor({}, 'primary')
	const iconColor = useThemeColor({}, 'icon')
	const textSecondary = useThemeColor({}, 'textSecondary')
	const errorColor = useThemeColor({}, 'error')

	const backdropOpacity = useRef(new Animated.Value(0)).current
	const cardScale = useRef(new Animated.Value(0.92)).current
	const cardOpacity = useRef(new Animated.Value(0)).current

	const [phase, setPhase] = useState<ConnectionState | 'empty' | 'permError'>(
		'idle',
	)
	const [devices, setDevices] = useState<BLEDevice[]>([])
	const [connectingId, setConnectingId] = useState<string | null>(null)
	const [errorMsg, setErrorMsg] = useState<string | null>(null)

	useEffect(() => {
		if (visible) {
			Animated.parallel([
				Animated.timing(backdropOpacity, {
					toValue: 1,
					duration: 250,
					useNativeDriver: true,
				}),
				Animated.spring(cardScale, {
					toValue: 1,
					friction: 8,
					tension: 100,
					useNativeDriver: true,
				}),
				Animated.timing(cardOpacity, {
					toValue: 1,
					duration: 300,
					useNativeDriver: true,
				}),
			]).start()
		} else {
			Animated.parallel([
				Animated.timing(backdropOpacity, {
					toValue: 0,
					duration: 200,
					useNativeDriver: true,
				}),
				Animated.timing(cardScale, {
					toValue: 0.92,
					duration: 200,
					useNativeDriver: true,
				}),
				Animated.timing(cardOpacity, {
					toValue: 0,
					duration: 200,
					useNativeDriver: true,
				}),
			]).start()
		}
	}, [visible, backdropOpacity, cardScale, cardOpacity])

	useEffect(() => {
		if (!visible) return

		let cancelled = false

		const unsub = bleService.onEvent((event) => {
			if (cancelled) return

			switch (event.type) {
				case 'deviceDiscovered':
					setDevices((prev) => {
						if (prev.find((d) => d.id === event.device.id)) return prev
						return [...prev, event.device]
					})
					break

				case 'scanComplete':
					if (event.count === 0) {
						setPhase('empty')
					} else {
						setPhase('idle')
					}
					break

				case 'connectionStateChange':
					if (event.state === 'connected') {
						const dev = bleService.connectedDevice
						if (dev) onConnected(dev)
						onClose()
					} else if (event.state === 'failed') {
						setConnectingId(null)
						setPhase('idle')
					} else if (event.state === 'disconnected') {
						setConnectingId(null)
					}
					break

				case 'error':
					setErrorMsg(event.message)
					break
			}
		})

		;(async () => {
			const ok = await bleService.requestPermissions()
			if (!ok) {
				setPhase('permError')
				return
			}
			setPhase('scanning')
			setDevices([])
			setErrorMsg(null)
			bleService.startScan()
		})()

		return () => {
			cancelled = true
			unsub()
			bleService.stopScan()
		}
	}, [visible, onClose, onConnected])

	const handleConnect = useCallback(
		async (device: BLEDevice) => {
			setConnectingId(device.id)
			setErrorMsg(null)
			try {
				await bleService.connect(device.id)
				await StorageService.setLastDeviceId(device.id)
			} catch {}
		},
		[],
	)

	const handleClose = () => {
		setTimeout(onClose, 220)
	}

	const handleRetry = () => {
		setPhase('scanning')
		setDevices([])
		setErrorMsg(null)
		bleService.startScan()
	}

	return (
		<Modal
			visible={visible}
			transparent
			animationType='none'
			onRequestClose={handleClose}>
			<Animated.View style={[modalStyles.backdrop, { opacity: backdropOpacity }]}>
				<Pressable style={modalStyles.backdropTouch} onPress={handleClose} />

				<Animated.View
					style={[
						modalStyles.card,
						{ borderColor },
						{
							opacity: cardOpacity,
							transform: [{ scale: cardScale }],
						},
					]}>
					<View style={modalStyles.header}>
						<ThemedText
							type='subtitle'
							style={{ fontFamily: 'Montserrat_500Medium' }}>
							Connect to Display
						</ThemedText>
						<Pressable
							onPress={handleClose}
							style={modalStyles.closeBtn}
							hitSlop={8}>
							<MaterialIcons name='close' size={22} color={iconColor} />
						</Pressable>
					</View>

					<View style={modalStyles.body}>
						{phase === 'permError' && (
							<View style={modalStyles.scanningWrap}>
								<MaterialIcons
									name='security'
									size={40}
									color={errorColor}
								/>
								<ThemedText
									style={{
										color: textSecondary,
										fontSize: 14,
										marginTop: 12,
										textAlign: 'center',
									}}>
									Bluetooth permissions are required to scan for devices.
								</ThemedText>
							</View>
						)}

						{phase === 'scanning' && (
							<View style={modalStyles.scanningWrap}>
								<Animated.View
									style={[
										modalStyles.scanIconCircle,
										{
											backgroundColor: primaryColor + '15',
											borderColor: primaryColor + '30',
										},
									]}>
									<MaterialIcons
										name='bluetooth-searching'
										size={28}
										color={primaryColor}
									/>
								</Animated.View>
								<ThemedText
									style={{
										color: textSecondary,
										fontSize: 14,
										marginTop: 12,
									}}>
									Searching for Round Displays...
								</ThemedText>
								{devices.length > 0 && (
									<ThemedText
										style={{
											color: primaryColor,
											fontSize: 12,
											marginTop: 4,
										}}>
										{devices.length} found so far
									</ThemedText>
								)}
							</View>
						)}

						{phase === 'empty' && (
							<View style={modalStyles.scanningWrap}>
								<MaterialIcons
									name='bluetooth-disabled'
									size={40}
									color={textSecondary}
								/>
								<ThemedText
									style={{
										color: textSecondary,
										fontSize: 14,
										marginTop: 12,
										textAlign: 'center',
									}}>
									No Round Displays found
								</ThemedText>
								<ThemedText
									style={{
										color: textSecondary,
										fontSize: 12,
										marginTop: 4,
										textAlign: 'center',
										paddingHorizontal: 20,
									}}>
									Make sure your display is powered on and nearby.
								</ThemedText>
								<Pressable onPress={handleRetry} style={modalStyles.retryBtn}>
									<ThemedText
										style={{
											color: primaryColor,
											fontSize: 13,
											fontFamily: 'Montserrat_600SemiBold',
											textTransform: 'uppercase',
										}}>
										Scan again
									</ThemedText>
								</Pressable>
							</View>
						)}

						{errorMsg && (
							<ThemedView
								color='errorContainer'
								style={modalStyles.errorBanner}>
								<MaterialIcons
									name='error-outline'
									size={16}
									color={errorColor}
								/>
								<ThemedText
									style={{
										color: errorColor,
										fontSize: 12,
										flex: 1,
									}}>
									{errorMsg}
								</ThemedText>
							</ThemedView>
						)}

						{devices.length > 0 && (
							<>
								<ThemedText
									style={{
										color: textSecondary,
										fontSize: typography.label.fontSize,
										fontFamily: typography.label.fontFamily,
										letterSpacing: typography.label.letterSpacing,
										textTransform: 'uppercase',
										paddingLeft: 4,
										marginBottom: 10,
									}}>
									Available Devices
								</ThemedText>

								{devices.map((device, i) => {
									const bars = rssiBars(device.rssi)
									const isConnecting = connectingId === device.id

									return (
										<AnimatedReanimated.View
											key={device.id}
											entering={FadeInUp.duration(400)
												.delay(i * 100)
												.springify()}>
											<Pressable
												onPress={() => handleConnect(device)}
												disabled={connectingId !== null}
												style={({ pressed }) => [
													modalStyles.deviceRow,
													{ borderColor },
													pressed && { opacity: 0.7 },
													isConnecting && {
														borderColor: primaryColor + '60',
													},
												]}>
												<ThemedView
													color='surfaceHigh'
													style={[
														modalStyles.deviceIconCircle,
														{ borderColor },
													]}>
													<MaterialIcons
														name='watch'
														size={22}
														color={iconColor}
													/>
												</ThemedView>

												<View style={modalStyles.deviceInfo}>
													<ThemedText
														type='defaultSemiBold'
														style={{ fontSize: 14 }}>
														{device.name}
													</ThemedText>
													<ThemedText
														style={{
															fontSize: 11,
															color: textSecondary,
														}}>
														{device.rssi} dBm
													</ThemedText>
												</View>

												{isConnecting ? (
													<ActivityIndicator
														size='small'
														color={primaryColor}
													/>
												) : (
													<View style={modalStyles.rssiWrap}>
														{[1, 2, 3, 4].map((bar) => (
															<View
																key={bar}
																style={[
																	modalStyles.rssiBar,
																	{
																		backgroundColor:
																			bar <= bars
																				? primaryColor
																				: borderColor,
																		height: 4 + bar * 3,
																	},
																]}
															/>
														))}
													</View>
												)}
											</Pressable>
										</AnimatedReanimated.View>
									)
								})}
							</>
						)}
					</View>
				</Animated.View>
			</Animated.View>
		</Modal>
	)
}

const modalStyles = StyleSheet.create({
	backdrop: {
		flex: 1,
		backgroundColor: 'rgba(0,0,0,0.55)',
		justifyContent: 'center',
		alignItems: 'center',
		paddingHorizontal: 24,
	},
	backdropTouch: {
		...StyleSheet.absoluteFillObject,
	},
	card: {
		width: '100%',
		maxWidth: 360,
		backgroundColor: '#1E2020',
		borderRadius: 24,
		borderWidth: 1,
		overflow: 'hidden',
	},
	header: {
		flexDirection: 'row',
		alignItems: 'center',
		justifyContent: 'space-between',
		paddingHorizontal: 20,
		paddingTop: 20,
		paddingBottom: 8,
	},
	closeBtn: {
		width: 32,
		height: 32,
		borderRadius: 16,
		alignItems: 'center',
		justifyContent: 'center',
	},
	body: {
		paddingHorizontal: 20,
		paddingBottom: 20,
		paddingTop: 4,
		minHeight: 200,
	},
	scanningWrap: {
		alignItems: 'center',
		justifyContent: 'center',
		paddingVertical: 32,
	},
	scanIconCircle: {
		width: 60,
		height: 60,
		borderRadius: 30,
		borderWidth: 1,
		alignItems: 'center',
		justifyContent: 'center',
	},
	retryBtn: {
		marginTop: 16,
		paddingVertical: 10,
		paddingHorizontal: 24,
		borderRadius: 20,
		borderWidth: 1,
		borderColor: '#434843',
	},
	errorBanner: {
		flexDirection: 'row',
		alignItems: 'center',
		gap: 8,
		paddingVertical: 8,
		paddingHorizontal: 12,
		borderRadius: 8,
		marginBottom: 12,
	},
	deviceRow: {
		flexDirection: 'row',
		alignItems: 'center',
		gap: 12,
		paddingVertical: 12,
		paddingHorizontal: 14,
		borderRadius: 14,
		borderWidth: 1,
		marginBottom: 8,
	},
	deviceIconCircle: {
		width: 44,
		height: 44,
		borderRadius: 22,
		borderWidth: 1,
		alignItems: 'center',
		justifyContent: 'center',
	},
	deviceInfo: {
		flex: 1,
	},
	rssiWrap: {
		flexDirection: 'row',
		alignItems: 'flex-end',
		gap: 2,
		height: 20,
	},
	rssiBar: {
		width: 4,
		borderRadius: 2,
	},
})
