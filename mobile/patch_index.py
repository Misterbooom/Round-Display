import re

with open('app/(tabs)/index.tsx', 'r') as f:
    content = f.read()

# Add Effect for debounced brightness
brightness_effect = """
	useEffect(() => {
		if (!isConnected) return;
		const id = setTimeout(() => {
			BleAPI.sendBrightness(brightness).catch(console.error);
		}, 500);
		return () => clearTimeout(id);
	}, [brightness, isConnected]);
"""

content = content.replace('const animOffset = useRef(new Animated.Value(CIRCUMFERENCE)).current', 'const animOffset = useRef(new Animated.Value(CIRCUMFERENCE)).current\n' + brightness_effect)


# Fix subscribeToBleEvents
new_subscribe = """	useEffect(() => {
		const unsubscribe = subscribeToBleEvents(
			(status) => {
				switch (status){
					case "disconnected":
						setIsConnected(false)
						console.log("device disconnected")
						break;
					case "connected":
						setIsConnected(true)
						console.log("device connected")
						// Initial sync upon connection
						BleAPI.sendTime(new Date().toISOString()).catch(console.error);
						getWeather("Gdansk").then(w => BleAPI.sendWeather(w)).then(() => setLastSynced(new Date())).catch(console.error);
						break;
				}
			},
			(batteryLevel) => {
				setBatteryPercentage(batteryLevel);
			},
			async () => {
				try {
					await BleAPI.sendTime(new Date().toISOString());
					await BleAPI.sendWeather(await getWeather("Gdansk"));
					setLastSynced(new Date());
				} catch (error) {
					console.error(error);
				}
			},
			() => {
			}
		);"""

content = re.sub(r'useEffect\(\(\) => \{\n\t\tconst unsubscribe = subscribeToBleEvents\([\s\S]*?(?=\}\);\n\n\t\treturn \(\) =>)', new_subscribe, content)

with open('app/(tabs)/index.tsx', 'w') as f:
    f.write(content)
