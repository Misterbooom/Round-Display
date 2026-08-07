import { ThemedText } from '@/components/themed-text'
import { typography } from '@/constants/theme'
import { useThemeColor } from '@/hooks/use-theme-color'

export function SectionLabel({ children }: { children: string }) {
	const textSecondary = useThemeColor({}, 'textSecondary')
	return (
		<ThemedText
			style={[
				{
					color: textSecondary,
					fontSize: typography.label.fontSize,
					fontFamily: typography.label.fontFamily,
					letterSpacing: typography.label.letterSpacing,
					textTransform: 'uppercase',
					paddingLeft: 4,
					marginBottom: 8,
				},
			]}>
			{children}
		</ThemedText>
	)
}
