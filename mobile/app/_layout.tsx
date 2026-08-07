import {
	Montserrat_100Thin,
	Montserrat_300Light,
	Montserrat_400Regular,
	Montserrat_500Medium,
	Montserrat_600SemiBold,
} from '@expo-google-fonts/montserrat'
import {
	DarkTheme,
	DefaultTheme,
	ThemeProvider,
} from '@react-navigation/native'
import { useFonts } from 'expo-font'
import { Stack } from 'expo-router'
import { StatusBar } from 'expo-status-bar'
import 'react-native-reanimated'

import { useColorScheme } from '@/hooks/use-color-scheme'
import { useInfoSync } from '@/hooks/useInfoSync'

export const unstable_settings = {
	anchor: '(tabs)',
}

export default function RootLayout() {
	const colorScheme = useColorScheme()

	useInfoSync()

	const [fontsLoaded] = useFonts({
		Montserrat_100Thin,
		Montserrat_300Light,
		Montserrat_400Regular,
		Montserrat_500Medium,
		Montserrat_600SemiBold,
	})

	if (!fontsLoaded) return null

	return (
		<ThemeProvider value={colorScheme === 'dark' ? DarkTheme : DefaultTheme}>
			<Stack>
				<Stack.Screen name='(tabs)' options={{ headerShown: false }} />
				<Stack.Screen
					name='modal'
					options={{ presentation: 'modal', title: 'Modal' }}
				/>
			</Stack>
			<StatusBar style='auto' />
		</ThemeProvider>
	)
}
