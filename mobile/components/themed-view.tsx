import { View, type ViewProps } from 'react-native'

import { type ColorName } from '@/constants/theme'
import { useThemeColor } from '@/hooks/use-theme-color'

export type ThemedViewProps = ViewProps & {
	lightColor?: string
	darkColor?: string
	color?: ColorName
}

export function ThemedView({
	style,
	lightColor,
	darkColor,
	color = 'background',
	...props
}: ThemedViewProps) {
	const backgroundColor = useThemeColor(
		{ light: lightColor, dark: darkColor },
		color,
	)

	return <View style={[{ backgroundColor }, style]} {...props} />
}
