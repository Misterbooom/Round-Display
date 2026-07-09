import { Text, type TextProps } from 'react-native'

import { typography } from '@/constants/theme'
import { useThemeColor } from '@/hooks/use-theme-color'

export type ThemedTextProps = TextProps & {
	lightColor?: string
	darkColor?: string
	type?: 'default' | 'title' | 'defaultSemiBold' | 'subtitle' | 'link'
}

/** Maps ThemedText `type` to the matching typography preset from the theme. */
const typeStyles: Record<NonNullable<ThemedTextProps['type']>, object> = {
	default: typography.body,
	title: typography.h1,
	defaultSemiBold: { ...typography.body, fontWeight: '600' as const },
	subtitle: typography.title,
	link: { ...typography.body },
}

export function ThemedText({
	style,
	lightColor,
	darkColor,
	type = 'default',
	...rest
}: ThemedTextProps) {
	const colorName = type === 'link' ? 'primary' : 'text'
	const color = useThemeColor({ light: lightColor, dark: darkColor }, colorName)

	return <Text style={[{ color }, typeStyles[type], style]} {...rest} />
}
