require('dotenv').config();
const express = require('express');
const mqtt = require('mqtt');
const cors = require('cors');

const app = express();
const PORT = process.env.PORT || 3000;

// Middleware
app.use(cors());
app.use(express.json());

// ==========================================
// MQTT Configuration
// ==========================================
const MQTT_BROKER = process.env.MQTT_BROKER || '97f6bb83bebf454c86a437210b5379b9.s1.eu.hivemq.cloud';
const MQTT_PORT = process.env.MQTT_PORT || 8883;
const MQTT_USERNAME = process.env.MQTT_USERNAME || 'anand';
const MQTT_PASSWORD = process.env.MQTT_PASSWORD || 'anand@1234';
const MQTT_TOPIC = process.env.MQTT_TOPIC || 'aeroshield/sensors/#';

// MQTT Client
let mqttClient = null;
let sensorData = {
  realtime: null,
  history: []
};

// Connect to MQTT Broker
function connectMQTT() {
  const options = {
    host: MQTT_BROKER,
    port: MQTT_PORT,
    username: MQTT_USERNAME,
    password: MQTT_PASSWORD,
    protocol: 'mqtts', // Secure MQTT
    rejectUnauthorized: false, // For development - remove in production
    connectTimeout: 30000, // 30 second connection timeout
    keepalive: 60,
    reconnectPeriod: 5000 // Reconnect every 5 seconds
  };

  console.log(`🔌 Connecting to MQTT Broker: ${MQTT_BROKER}:${MQTT_PORT}`);
  mqttClient = mqtt.connect(options);

  mqttClient.on('connect', () => {
    console.log('✅ Connected to MQTT Broker');
    mqttClient.subscribe(MQTT_TOPIC, (err) => {
      if (!err) {
        console.log(`📡 Subscribed to topic: ${MQTT_TOPIC}`);
      } else {
        console.error('❌ Subscription error:', err);
      }
    });
  });

  mqttClient.on('message', (topic, message) => {
    try {
      const data = JSON.parse(message.toString());
      console.log(`📥 Received data from ${topic}:`, data);
      
      // Update realtime data
      sensorData.realtime = data;
      
      // Add to history (keep last 100 records)
      sensorData.history.push(data);
      if (sensorData.history.length > 100) {
        sensorData.history.shift();
      }
    } catch (error) {
      console.error('❌ Error parsing MQTT message:', error);
    }
  });

  mqttClient.on('error', (err) => {
    console.error('❌ MQTT Error:', err.message);
  });

  mqttClient.on('reconnect', () => {
    console.log('🔄 Reconnecting to MQTT Broker...');
  });

  mqttClient.on('close', () => {
    console.log('❌ MQTT Connection closed');
  });

  mqttClient.on('offline', () => {
    console.log('⚠️ MQTT Client offline');
  });
}

// Initialize MQTT connection
connectMQTT();

// ==========================================
// REST API Endpoints
// ==========================================

// Health check
app.get('/health', (req, res) => {
  res.json({ 
    status: 'ok', 
    mqtt: mqttClient ? mqttClient.connected ? 'connected' : 'disconnected' : 'not initialized'
  });
});

// Get realtime sensor data
app.get('/api/sensors/realtime', (req, res) => {
  try {
    if (sensorData.realtime) {
      res.json(sensorData.realtime);
    } else {
      // Return default placeholder data when no sensor data available
      const defaultData = {
        temperature: 25.0,
        humidity: 60,
        ppm: 150,
        aqi: 73,
        category: 'Moderate',
        timestamp: Math.floor(Date.now() / 1000),
        city: 'chikodi',
        status: 'waiting_for_sensor_data'
      };
      console.log('📋 Returning default data (no sensor data yet)');
      res.json(defaultData);
    }
  } catch (error) {
    console.error('Error in realtime endpoint:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// Get historical sensor data
app.get('/api/sensors/history', (req, res) => {
  try {
    const limit = parseInt(req.query.limit) || 30;
    const history = sensorData.history.slice(-limit);
    res.json(history);
  } catch (error) {
    console.error('Error in history endpoint:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// Get sensor data by city (for compatibility with existing frontend)
app.get('/api/cities/:city/realtime', (req, res) => {
  try {
    if (sensorData.realtime) {
      res.json(sensorData.realtime);
    } else {
      // Return last known data or default values
      res.json({
        temperature: 0,
        humidity: 0,
        ppm: 0,
        aqi: 0,
        category: 'Good',
        timestamp: Math.floor(Date.now() / 1000),
        city: req.params.city,
        status: 'waiting_for_sensor_data'
      });
    }
  } catch (error) {
    console.error('Error in city realtime endpoint:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// Get historical data by city (for compatibility with existing frontend)
app.get('/api/cities/:city/history', (req, res) => {
  try {
    const limit = parseInt(req.query.limit) || 30;
    const history = sensorData.history.slice(-limit);
    
    // Convert array to object format for frontend compatibility
    const historyObj = {};
    history.forEach((record, index) => {
      const timestamp = record.timestamp || Math.floor(Date.now() / 1000) - (history.length - index);
      historyObj[timestamp] = record;
    });
    
    res.json(historyObj);
  } catch (error) {
    console.error('Error in city history endpoint:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// ==========================================
// Start Server
// ==========================================
app.listen(PORT, () => {
  console.log(`🚀 AeroShield Backend Server running on port ${PORT}`);
  console.log(`📡 MQTT Broker: ${MQTT_BROKER}:${MQTT_PORT}`);
  console.log(`👤 MQTT Username: ${MQTT_USERNAME}`);
  console.log(`🎯 MQTT Topic: ${MQTT_TOPIC}`);
  console.log(`🌐 Health check: http://localhost:${PORT}/health`);
  
  // Log Render environment info
  if (process.env.RENDER) {
    console.log(`🎭 Running on Render`);
    console.log(`🌍 Region: ${process.env.RENDER_REGION || 'unknown'}`);
  }
});
