# AeroShield - IoT Air Quality Monitor

Real-time air quality monitoring system using ESP8266, MQTT, and a modern web dashboard.

## Architecture

```
ESP8266 (Sensors) → MQTT (HiveMQ Cloud) → Node.js Backend → REST API → Frontend Dashboard
```

## Project Structure

```
aeroshield-project/
├── backend/          # Node.js MQTT broker & REST API server
├── frontend/         # React/Vanilla JS web dashboard
├── firmware/         # ESP8266 Arduino firmware
└── README.md
```

## Components

### Hardware
- **NodeMCU (ESP8266)**: Microcontroller
- **DHT11**: Temperature & Humidity sensor (Pin D4)
- **MQ135**: Air quality/Gas sensor (Pin A0)

### Software
- **Backend**: Node.js with Express and MQTT client
- **Frontend**: HTML/CSS/JS dashboard with Chart.js
- **Firmware**: Arduino C++ with WiFiManager and PubSubClient

### Cloud Services
- **MQTT Broker**: HiveMQ Cloud
- **Backend Hosting**: Render
- **Frontend Hosting**: Vercel

## Setup Instructions

### 1. Backend Setup (Render)

1. Navigate to the backend directory:
   ```bash
   cd backend
   ```

2. Install dependencies:
   ```bash
   npm install
   ```

3. Configure environment variables in `.env`:
   ```
   PORT=3000
   MQTT_BROKER=97f6bb83bebf454c86a437210b5379b9.s1.eu.hivemq.cloud
   MQTT_PORT=8883
   MQTT_USERNAME=97f6bb83bebf454c86a437210b5379b9
   MQTT_PASSWORD=
   MQTT_TOPIC=aeroshield/sensors/#
   ```

4. Test locally:
   ```bash
   npm start
   ```

5. Deploy to Render:
   - Push code to GitHub
   - Connect repository to Render
   - Set environment variables in Render dashboard
   - Deploy

### 2. Frontend Setup (Vercel)

1. Configure API URL in frontend:
   - Open `frontend/app.js`
   - Update `DEFAULT_CONFIG.apiBaseUrl` with your Render backend URL
   - Or use the settings modal in the dashboard

2. Deploy to Vercel:
   - Push code to GitHub
   - Import project in Vercel
   - Set root directory to `frontend`
   - Deploy

### 3. Firmware Setup (ESP8266)

1. Install required Arduino libraries:
   - DHT sensor library (Adafruit)
   - Adafruit Unified Sensor
   - PubSubClient (Nick O'Leary)
   - WiFiManager (tzapu)

2. Configure board in Arduino IDE:
   - Add ESP8266 board manager URL
   - Install ESP8266 boards
   - Select "NodeMCU 1.0 (ESP-12E Module)"

3. Upload firmware:
   - Open `firmware/AeroShield_MQTT.ino`
   - Upload to NodeMCU

4. Configure WiFi:
   - Connect to "AeroShield-Setup" WiFi network
   - Password: aeroshield123
   - Configure your WiFi credentials
   - Configure city name
   - Device will restart and connect
   - MQTT credentials are pre-configured in firmware (anand / anand@1234)

## API Endpoints

### Health Check
```
GET /health
```

### Real-time Sensor Data
```
GET /api/sensors/realtime
GET /api/cities/{city}/realtime
```

### Historical Data
```
GET /api/sensors/history?limit=30
GET /api/cities/{city}/history?limit=30
```

## MQTT Topics

### Publishing (ESP8266 → Backend)
- Topic: `aeroshield/sensors/data`
- Format: JSON with temperature, humidity, ppm, aqi, category, timestamp, city

### Subscribing (Backend → ESP8266)
- Topic: `aeroshield/commands/#` (for future commands)

## Data Flow

1. **ESP8266** reads sensors every 10 seconds
2. **ESP8266** publishes JSON data to MQTT topic
3. **Backend** subscribes to MQTT topic and stores data
4. **Frontend** polls backend API every 5 seconds
5. **Dashboard** updates with real-time sensor readings

## Configuration

### MQTT Credentials
- Broker: `97f6bb83bebf454c86a437210b5379b9.s1.eu.hivemq.cloud`
- Port: `8883` (Secure MQTT)
- Username: `97f6bb83bebf454c86a437210b5379b9`
- Password: (empty)

### Sensor Pin Configuration
- DHT11: Pin D4
- MQ135: Pin A0

## Troubleshooting

### Backend Issues
- Check MQTT connection in backend logs
- Verify environment variables
- Ensure HiveMQ Cloud cluster is running

### Frontend Issues
- Check browser console for errors
- Verify API URL configuration
- Check CORS settings if using different domains

### Firmware Issues
- Use Serial Monitor (115200 baud) for debugging
- Ensure proper sensor connections
- Allow 2-3 minutes for MQ135 warm-up
- Reconfigure WiFi via captive portal if needed

## Features

- Real-time air quality monitoring
- Temperature and humidity tracking
- Historical data visualization
- WiFi configuration via captive portal
- Secure MQTT communication
- Responsive web dashboard
- Demo mode for testing without hardware

## License

MIT License - Feel free to use and modify for your projects.
