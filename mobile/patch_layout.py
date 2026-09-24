import re

with open('app/(tabs)/_layout.tsx', 'r') as f:
    content = f.read()

dev_tab = """				<Tabs.Screen
					name='developer'
					options={{
						title: 'Developer',
						tabBarIcon: ({ color }) => (
							<IconSymbol size={28} name='chevron.left.forwardslash.chevron.right' color={color} />
						),
					}}
				/>
"""

content = content.replace('</Tabs>', dev_tab + '\t\t\t</Tabs>')

with open('app/(tabs)/_layout.tsx', 'w') as f:
    f.write(content)
