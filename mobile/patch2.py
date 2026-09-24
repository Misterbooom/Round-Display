import re

with open('app/(tabs)/index.tsx', 'r') as f:
    content = f.read()

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

# Replace the block
content = re.sub(
    r'([ \t]*)useEffect\(\(\) => \{\n[ \t]*const unsubscribe = subscribeToBleEvents\([\s\S]*?\(\) => \{\n[ \t]*\}\n[ \t]*\);',
    new_subscribe,
    content
)

with open('app/(tabs)/index.tsx', 'w') as f:
    f.write(content)
