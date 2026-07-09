import { Tabs } from 'expo-router'
import React from 'react'
import { StyleSheet } from 'react-native'

import { HapticTab } from '@/components/haptic-tab'
import { ThemedText } from '@/components/themed-text'
import { ThemedView } from '@/components/themed-view'
import { IconSymbol } from '@/components/ui/icon-symbol'
import { colors, typography } from '@/constants/theme'
import { useColorScheme } from '@/hooks/use-color-scheme'
import { useThemeColor } from '@/hooks/use-theme-color'
export default function TabLayout() {
	const colorScheme = useColorScheme() ?? 'dark'
	const borderColor = useThemeColor({}, 'border')
	const primaryColor = useThemeColor({}, 'primary')
	return (
		<>
			<ThemedView
				accessible={true}
				accessibilityRole='header'
				style={[styles.header, { borderBottomColor: borderColor }]}>
				<ThemedText
					type='title'
					style={[
						styles.title,

						{
							color: primaryColor,
							fontFamily: typography.h2.fontFamily,
							letterSpacing: -1,
						},
					]}>
					Round Display
				</ThemedText>
			</ThemedView>

			<Tabs
				screenOptions={{
					tabBarActiveTintColor: colors[colorScheme].primary,
					headerShown: false,
					tabBarButton: HapticTab,
				}}>
				<Tabs.Screen
					name='index'
					options={{
						title: 'Home',
						tabBarIcon: ({ color }) => (
							<IconSymbol size={28} name='house.fill' color={color} />
						),
					}}
				/>
				<Tabs.Screen
					name='connect'
					options={{
						title: 'Connect',
						tabBarIcon: ({ color }) => (
							<IconSymbol size={28} name='antenna.radiowaves.left.and.right' color={color} />
						),
					}}
				/>
			</Tabs>
		</>
	)
}

const styles = StyleSheet.create({
	header: {
		display: 'flex',
		alignItems: 'center',
		paddingTop: 60,
		paddingBottom: 16,
		borderBottomWidth: 1,
	},
	title: {
		textTransform: 'uppercase',
		textAlign: 'center',
	},
})
