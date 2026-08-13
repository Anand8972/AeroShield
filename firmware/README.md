# AeroShield ESP8266 Firmware

This firmware is for the NodeMCU (ESP8266) microcontroller that reads sensor data and publishes it to the HiveMQ MQTT broker.

## Hardware Components

- **Microcontroller**: NodeMCU (ESP8266)
- **Temperature & Humidity Sensor**: DHT11
- **Air Quality Sensor**: MQ135
- **MQTT Broker**: HiveMQ Cloud

## Pin Connections

- **DHT11 Data Pin**: D4
- **MQ135 Analog Pin**: A0

## Required Libraries

Install these libraries via Arduino Library Manager:

1. "DHT sensor library" by Adafruit
2. "Adafruit Unified Sensor" (automatically prompted by DHT library)
3. "PubSubClient" by Nick O'Leary
4. "WiFiManager" by tzapu

## Board Configuration

1. Add this URL to Arduino IDE Preferences (Additional Boards Manager URLs):
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```

2. In Boards Manager, install "esp8266" by ESP8266 Community

3. Select Board: "NodeMCU 1.0 (ESP-12E Module)"

## WiFi Configuration

The firmware uses WiFiManager to create a captive portal for configuration:

1. Upload the firmware to your NodeMCU
2. Connect to the WiFi network "AeroShield-Setup" (password: aeroshield123)
3. A configuration portal will open automatically
4. Select your WiFi network and enter password
5. Configure your city name
6. **Configure MQTT username and password** (default: anand / anand@1234)
7. The device will restart and connect to your WiFi

## MQTT Configuration

The firmware is pre-configured with your HiveMQ Cloud credentials:

- **Broker**: 97f6bb83bebf454c86a437210b5379b9.s1.eu.hivemq.cloud
- **Port**: 8883 (Secure MQTT)
- **Username**: 97f6bb83bebf454c86a437210b5379b9
- **Topic**: aeroshield/sensors/data

## Data Format

The device publishes JSON data to the MQTT topic every 10 seconds:

```json
{
  "temperature": 28.5,
  "humidity": 65,
  "ppm": 210,
  "aqi": 102,
  "category": "Moderate",
  "timestamp": 1692345678,
  "city": "chikodi"
}
```

## Troubleshooting

### WiFi Connection Issues
- If the device can't connect to WiFi, it will create the "AeroShield-Setup" access point
- Connect to the AP and reconfigure your WiFi credentials

### MQTT Connection Issues
- Check that your HiveMQ Cloud cluster is running
- Verify the broker address and credentials in the code
- The device will retry MQTT connection automatically

### Sensor Reading Issues
- Ensure DHT11 is connected to pin D4
- Ensure MQ135 is connected to pin A0
- Check power supply to sensors
- Allow 2-3 minutes for MQ135 to warm up for accurate readings

## Serial Monitor

Open Serial Monitor at 115200 baud to see:
- WiFi connection status
- MQTT connection status
- Sensor readings
- Data publication confirmation
