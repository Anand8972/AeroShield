# AeroShield Project - Complete Project Description for Report Generation

## Project Overview
**Project Name**: AeroShield - IoT Air Quality & Climate Monitoring System
**Type**: College Major Project
**Purpose**: Real-time air quality monitoring with ESP8266 sensors, MQTT data transmission, cloud backend, and web dashboard

## Technologies Used

### Hardware
- **Microcontroller**: ESP8266 NodeMCU (ESP-12E Module)
- **Sensors**:
  - DHT11 (Temperature & Humidity sensor) - Connected to D4 pin
  - MQ135 (Gas/Air Quality sensor) - Analog output connected to A0 pin
- **Communication**: WiFi (802.11 b/g/n), MQTT over TLS

### Software & Technologies
- **Firmware**: Arduino C++ (ESP8266 core 3.1.2)
- **Backend**: Node.js with Express.js
- **Database**: In-memory (no persistent database - real-time data flow)
- **Messaging Protocol**: MQTT (Message Queuing Telemetry Transport)
- **Cloud Services**:
  - MQTT Broker: HiveMQ Cloud (Free tier)
  - Backend Hosting: Render (Free tier)
  - Frontend Hosting: Vercel (Free tier)
- **Frontend**: HTML5, CSS3, Vanilla JavaScript, Chart.js
- **Libraries**:
  - Arduino: PubSubClient, WiFiManager, DHT sensor library, Adafruit Unified Sensor
  - Node.js: mqtt, express, cors, dotenv
  - Frontend: Chart.js, FontAwesome

## System Architecture

### Data Flow
```
ESP8266 + DHT11 + MQ135
    ↓ (WiFi)
WiFiManager (AP for WiFi credentials)
    ↓ (MQTT over TLS - Port 8883)
HiveMQ Cloud Broker
    ↓ (MQTT Subscription)
Node.js Backend (Render)
    ↓ (REST API)
Web Dashboard (Vercel)
```

### Components

#### 1. ESP8266 Firmware (AeroShield.ino)
- **Purpose**: Read sensors and publish data to MQTT
- **Key Features**:
  - WiFiManager for easy WiFi configuration via Access Point
  - Auto-reopen AP after 5 seconds of WiFi disconnect
  - Custom styled portal (purple-blue gradient UI)
  - MQTT over TLS connection to HiveMQ
  - DHT11 temperature/humidity reading
  - MQ135 analog reading and AQI calculation
  - JSON payload publishing every 10 seconds
  - Auto-reconnection for both WiFi and MQTT
- **MQTT Topic**: `aeroshield/sensors/data`
- **MQTT Broker**: `97f6bb83bebf454c86a437210b5379b9.s1.eu.hivemq.cloud:8883`
- **Credentials**: Username: `anand`, Password: `anand@1234`

#### 2. Node.js Backend (backend/server.js)
- **Purpose**: Subscribe to MQTT, store data, serve REST API
- **Key Features**:
  - Express.js REST API server
  - MQTT client subscribing to `aeroshield/sensors/#`
  - In-memory data storage (realtime + history)
  - CORS enabled for frontend
  - Health check endpoint
  - Default data when MQTT not connected
  - Auto-reconnection to MQTT broker
- **API Endpoints**:
  - `GET /health` - Health check with MQTT status
  - `GET /api/sensors/realtime` - Latest sensor reading
  - `GET /api/sensors/history` - Historical data (limit query param)
  - `GET /api/cities/:city/realtime` - City-specific realtime data
  - `GET /api/cities/:city/history` - City-specific historical data

#### 3. Web Dashboard (frontend/)
- **Purpose**: Display real-time air quality data
- **Key Features**:
  - Modern glassmorphism UI with gradient backgrounds
  - Real-time AQI display with color-coded categories
  - Temperature and humidity metrics
  - MQ135 gas concentration (PPM)
  - Interactive charts using Chart.js
  - Auto-refresh every 5 seconds
  - Responsive design (mobile, tablet, desktop)
  - Settings modal for API configuration
  - Connection status indicator
- **UI/UX**:
  - Purple-blue gradient background
  - Glassmorphism cards with blur effects
  - Smooth animations and transitions
  - Mobile-first responsive design
  - Professional typography (Space Grotesk, Outfit fonts)

## AQI Calculation System

### Formula
```cpp
int aqi = map(rawValue, 0, 1023, 0, 500);
```

### Categories
- **0-50**: Good (Green)
- **51-100**: Moderate (Yellow)
- **101-150**: USG - Unhealthy for Sensitive Groups (Orange)
- **151-200**: Unhealthy (Red)
- **201-300**: Very Unhealthy (Purple)
- **>300**: Hazardous (Maroon)

## WiFi Configuration System

### Access Point Details
- **SSID**: `AeroShield-Setup`
- **Password**: `aeroshield123`
- **Purpose**: Allow users to enter WiFi credentials without recompiling firmware

### WiFi Features
1. **Initial Setup**: On first boot, ESP8266 creates AP
2. **User Action**: Connect to AP, enter WiFi SSID/password in browser
3. **Auto-Reconnect**: Credentials saved, ESP8266 auto-connects on boot
4. **Auto-Reopen AP**: If WiFi disconnects for 5+ seconds, AP reopens automatically
5. **Custom Portal UI**: Modern gradient design, only SSID/Password fields

## Deployment Architecture

### Backend (Render)
- **URL**: https://aeroshield.onrender.com
- **Environment Variables**:
  - `PORT=3000`
  - `MQTT_BROKER=97f6bb83bebf454c86a437210b5379b9.s1.eu.hivemq.cloud`
  - `MQTT_PORT=8883`
  - `MQTT_USERNAME=anand`
  - `MQTT_PASSWORD=anand@1234`
  - `MQTT_TOPIC=aeroshield/sensors/#`

### Frontend (Vercel)
- **Configuration**: Root directory is `frontend/`
- **API URL**: Configured to point to Render backend
- **Static Hosting**: Serves HTML, CSS, JS, Chart.js

### Firmware (Local)
- **File**: `AeroShield.ino` (single file in project root)
- **Board**: NodeMCU 1.0 (ESP-12E Module)
- **Upload**: Arduino IDE

## Project Structure
```
aeroshield-project/
├── AeroShield.ino              # ESP8266 firmware (single file)
├── backend/                    # Node.js backend
│   ├── server.js              # Express + MQTT server
│   ├── package.json           # Node.js dependencies
│   ├── .env                   # Environment variables (not in git)
│   └── .env.example          # Environment variables template
├── frontend/                   # Web dashboard
│   ├── index.html             # Main HTML page
│   ├── app.js                 # Frontend JavaScript
│   ├── style.css              # Styling (modern glassmorphism)
│   ├── package.json           # npm dependencies
│   └── vercel.json            # Vercel configuration
├── README.md                  # Project documentation
└── DEPLOYMENT.md              # Deployment guide
```

## Key Features Summary

### For End Users
- Real-time air quality monitoring
- Easy WiFi setup via Access Point
- Modern, responsive web dashboard
- Mobile-friendly interface
- Color-coded AQI categories
- Historical data visualization with charts

### For Developers
- MQTT-based architecture (scalable)
- No database (real-time data flow)
- Easy deployment (Render + Vercel)
- Clear project structure
- Well-documented code
- Modern UI/UX design

## Technical Challenges Solved

1. **WiFiManager API Compatibility**: Fixed deprecated methods for version 2.0+
2. **MQTT TLS Connection**: Configured secure MQTT connection
3. **Auto-Reconnection**: Implemented WiFi and MQTT auto-reconnect
4. **Access Point Automation**: Auto-reopen AP after WiFi disconnect
5. **UI Responsiveness**: Full mobile/tablet/desktop support
6. **Backend Availability**: Default data when MQTT not connected
7. **Vercel CSS Loading**: Fixed static file serving

## Future Enhancements (Potential)

1. **Database Integration**: Add MongoDB/PostgreSQL for persistent storage
2. **Multiple Sensors**: Support for additional sensor types
3. **Alert System**: Email/SMS notifications for dangerous AQI levels
4. **User Authentication**: Multi-user support with login
5. **Historical Analytics**: Advanced data analysis and reporting
6. **Mobile App**: Native Android/iOS application
7. **Multiple Locations**: Support for multiple sensor nodes
8. **Calibration**: Proper MQ135 calibration for accurate readings

## Report Sections Suggested

1. **Introduction**
   - Problem statement
   - Objectives
   - Scope

2. **Literature Review**
   - IoT in environmental monitoring
   - Air quality indices
   - MQTT protocol advantages

3. **System Design**
   - Architecture diagram
   - Component selection
   - Data flow

4. **Implementation**
   - Hardware setup
   - Firmware development
   - Backend development
   - Frontend development

5. **Testing & Results**
   - Sensor accuracy
   - System reliability
   - Performance metrics

6. **Conclusion**
   - Achievements
   - Limitations
   - Future scope

## Key Technical Terms to Include

- **IoT (Internet of Things)**
- **MQTT (Message Queuing Telemetry Transport)**
- **ESP8266**
- **NodeMCU**
- **DHT11**
- **MQ135**
- **AQI (Air Quality Index)**
- **TLS (Transport Layer Security)**
- **Access Point (AP)**
- **WiFiManager**
- **REST API**
- **Real-time Data**
- **Cloud Computing**
- **Microservices Architecture**
- **Glassmorphism UI**
- **Responsive Design**

## Statistics

- **Development Time**: Several sessions
- **Lines of Code**: ~800+ (across all components)
- **Sensors**: 2 (DHT11, MQ135)
- **API Endpoints**: 5
- **MQTT Topics**: 1 (with wildcard subscription)
- **Supported Devices**: Mobile, Tablet, Desktop
- **Data Update Rate**: Every 10 seconds (firmware) / 5 seconds (frontend polling)

## Contact/Repository

- **GitHub**: https://github.com/Anand8972/AeroShield.git
- **Backend URL**: https://aeroshield.onrender.com
- **Frontend URL**: (Your Vercel deployment URL)

---

**Note for GPT**: Use this comprehensive information to generate a detailed, professional project report suitable for college submission. Include diagrams, code snippets, and technical explanations where appropriate.
