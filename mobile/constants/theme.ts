// ── Color palettes ──────────────────────────────────────────
const darkColors = {
	// Base
	background: '#121414',
	surface: '#121414',

	// Containers
	surfaceLowest: '#0D0E0F',
	surfaceLow: '#1A1C1C',
	surfaceContainer: '#1E2020',
	surfaceHigh: '#282A2B',
	surfaceHighest: '#333535',

	// Primary
	primary: '#B8CCB8',
	primaryContainer: '#1B2B1E',
	primaryFixed: '#D4E8D4',

	// Secondary
	secondary: '#B2CEB4',
	secondaryContainer: '#364F3B',

	// Tertiary
	tertiary: '#C1C9BF',
	tertiaryContainer: '#222922',

	// Text
	text: '#E2E2E2',
	textSecondary: '#C3C8C0',
	textMuted: '#899087',

	// Borders
	border: '#434843',
	borderStrong: '#8D928B',

	// Status
	success: '#B8CCB8',
	error: '#FFB4AB',
	errorContainer: '#93000A',

	// Misc
	shadow: 'rgba(27,43,30,0.2)',
	icon: '#C3C8C0',
} as const

const lightColors = {
	// Base
	background: '#F8FAF8',
	surface: '#F8FAF8',

	// Containers
	surfaceLowest: '#FFFFFF',
	surfaceLow: '#F0F2EF',
	surfaceContainer: '#EBEDEA',
	surfaceHigh: '#E1E4E0',
	surfaceHighest: '#D8DBD6',

	// Primary
	primary: '#2D4A30',
	primaryContainer: '#D4E8D4',
	primaryFixed: '#1B2B1E',

	// Secondary
	secondary: '#3A553E',
	secondaryContainer: '#D0E6D2',

	// Tertiary
	tertiary: '#3A4038',
	tertiaryContainer: '#DDE3D9',

	// Text
	text: '#1A1C1A',
	textSecondary: '#444A43',
	textMuted: '#727970',

	// Borders
	border: '#C4C9C2',
	borderStrong: '#899087',

	// Status
	success: '#2D4A30',
	error: '#BA1A1A',
	errorContainer: '#FFDAD6',

	// Misc
	shadow: 'rgba(0,0,0,0.08)',
	icon: '#444A43',
} as const

export const colors = {
	light: lightColors,
	dark: darkColors,
} as const

export type ColorName = keyof typeof darkColors

export const radius = {
	xs: 4,
	sm: 8,
	md: 12,
	lg: 16,
	xl: 24,
	full: 9999,
} as const

export const spacing = {
	xs: 4,
	sm: 8,
	md: 12,
	lg: 16,
	xl: 24,
	xxl: 40,
} as const
export const typography = {
	display: {
		fontFamily: 'Montserrat_300Light',
		fontSize: 48,
		fontWeight: '300' as const,
		lineHeight: 52,
	},
	h1: {
		fontFamily: 'Montserrat_500Medium',
		fontSize: 32,
		fontWeight: '500' as const,
		lineHeight: 40,
	},

	h2: {
		fontFamily: 'Montserrat_500Medium',
		fontSize: 24,
		fontWeight: '500' as const,
		lineHeight: 32,
	},

	title: {
		fontFamily: 'Montserrat',
		fontSize: 20,
		fontWeight: '500' as const,
	},

	body: {
		fontFamily: 'Montserrat',
		fontSize: 16,
		fontWeight: '400' as const,
		lineHeight: 24,
	},

	label: {
		fontFamily: 'Montserrat_600SemiBold',
		fontSize: 12,
		fontWeight: '600' as const,
		lineHeight: 16,
		letterSpacing: 0.6,
	},
} as const

export const shadows = {
	card: {
		shadowColor: '#1B2B1E',
		shadowOpacity: 0.2,
		shadowRadius: 20,
		shadowOffset: { width: 0, height: 0 },
		elevation: 5,
	},
} as const

export const theme = {
	colors,
	radius,
	spacing,
	typography,
	shadows,
} as const
