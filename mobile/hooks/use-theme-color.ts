import { useColorScheme } from 'react-native'

import { colors, type ColorName } from '@/constants/theme'

/**
 * Returns the active color for the current color scheme.
 *
 * @param props.light  — override the light-mode value for this call
 * @param props.dark   — override the dark-mode value for this call
 * @param colorName    — key into the theme palette (fallback when no override)
 */
export function useThemeColor(
	props: { light?: string; dark?: string },
	colorName: ColorName,
): string {
	const scheme = useColorScheme() ?? 'dark'

	if (scheme === 'light' && props.light !== undefined) return props.light
	if (scheme === 'dark' && props.dark !== undefined) return props.dark

	return colors[scheme][colorName]
}
