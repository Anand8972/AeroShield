/*
  AeroShield Air Quality & Climate Monitor
  Device: NodeMCU (ESP8266)
  Sensors: DHT11 (Temperature/Humidity) & MQ135 (Air Quality/PPM)
  Cloud Integration: HiveMQ MQTT Broker
  
  Dependencies (Install via Arduino Library Manager):
  1. "DHT sensor library" by Adafruit
  2. "Adafruit Unified Sensor" (automatically prompted by DHT library)
  3. "PubSubClient" by Nick O'Leary
  4. "WiFiManager" by tzapu
  
  Board Manager URL (in Preferences):
  http://arduino.esp8266.com/stable/package_esp8266com_index.json
  Board Select: "NodeMCU 1.0 (ESP-12E Module)"
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFiManager.h>
#include <time.h>

// ==========================================
// MQTT Configuration
// ==========================================
const char* MQTT_BROKER = "97f6bb83bebf454c86a437210b5379b9.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
const char* MQTT_TOPIC = "aeroshield/sensors/data";
const char* MQTT_CLIENT_ID = "AeroShield-ESP8266";

// ==========================================
// Hardware Pins Definition
// ==========================================
#define DHTPIN D4         // DHT11 Data Pin connected to D4
#define DHTTYPE DHT11     // DHT sensor type (DHT11)
#define MQ135_PIN A0      // MQ135 Analog Pin connected to A0

// Initialize DHT Sensor
DHT dht(DHTPIN, DHTTYPE);

// WiFi and MQTT Clients
WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);
WiFiManager wifiManager;

// Configuration
String CITY = "chikodi";
String MQTT_USER = "anand";
String MQTT_PASS = "anand@1234";
unsigned long lastMillis = 0;
const long INTERVAL = 10000; // Send data every 10 seconds

// ==========================================
// Helper: Map AQI to human-readable category
// ==========================================
String categoryAQI(int aqi)
{
  if (aqi <= 50) return "Good";
  if (aqi <= 100) return "Moderate";
  if (aqi <= 150) return "USG"; // Unhealthy for Sensitive Groups
  if (aqi <= 200) return "Unhealthy";
  if (aqi <= 300) return "Very Unhealthy";
  return "Hazardous";
}

// ==========================================
// Helper: Map raw sensor readings (0-1023) to simple AQI estimate (0-500)
// ==========================================
int calculateAQI(int ppm)
{
  int aqi = map(ppm, 0, 1023, 0, 500);

  if (aqi < 0) aqi = 0;
  if (aqi > 500) aqi = 500;

  return aqi;
}

// ==========================================
// MQTT Callback
// ==========================================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming MQTT messages if needed
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// ==========================================
// MQTT Connection Routine
// ==========================================
void connectMQTT() {
  // For HiveMQ Cloud, we need to handle SSL/TLS
  espClient.setInsecure(); // Skip certificate validation for development
  
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  Serial.print("Connecting to MQTT Broker ");
  Serial.print(" as user: ");
  Serial.println(MQTT_USER);
  int attempts = 0;
  
  while (!mqttClient.connected() && attempts < 5) {
    Serial.print(".");
    attempts++;
    
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER.c_str(), MQTT_PASS.c_str())) {
      Serial.println("\n✅ Connected to MQTT Broker!");
      
      // Subscribe to topics if needed
      // mqttClient.subscribe("aeroshield/commands/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }

  if (!mqttClient.connected()) {
    Serial.println("❌ MQTT connection failed after multiple attempts");
  }
}

// ==========================================
// Setup
// ==========================================
void setup()
{
  Serial.begin(9600);
  delay(10);
  
  Serial.println("\n--- AeroShield Air Quality Monitor Startup ---");
  
  // Initialize Sensor
  dht.begin();
  
  // Initialize WiFiManager
  // WiFiManager will create an AP named "AeroShield-Setup" if no saved WiFi
  wifiManager.setAPName("AeroShield-Setup");
  wifiManager.setAPPassword("aeroshield123");
  
  // Custom parameters for city and MQTT configuration
  WiFiManagerParameter customCity("city", "City Name", CITY.c_str(), 20);
  WiFiManagerParameter customMqttUser("mqtt_user", "MQTT Username", MQTT_USER.c_str(), 30);
  WiFiManagerParameter customMqttPass("mqtt_pass", "MQTT Password", MQTT_PASS.c_str(), 30);
  
  wifiManager.addParameter(&customCity);
  wifiManager.addParameter(&customMqttUser);
  wifiManager.addParameter(&customMqttPass);
  
  // Try to connect to WiFi, or start AP if no saved credentials
  if (!wifiManager.autoConnect("AeroShield-Setup", "aeroshield123")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  Serial.println("✅ WiFi Connected successfully!");
  Serial.print("Local IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Get custom parameter values
  CITY = customCity.getValue();
  MQTT_USER = customMqttUser.getValue();
  MQTT_PASS = customMqttPass.getValue();
  
  Serial.print("City configured as: ");
  Serial.println(CITY);
  Serial.print("MQTT Username: ");
  Serial.println(MQTT_USER);
  
  // Connect to MQTT
  connectMQTT();

  // Synchronize Time via NTP (Indian Standard Time offset: UTC + 5.5 Hours = 19800 seconds)
  Serial.print("Synchronizing time with NTP server");
  configTime(19800, 0, "pool.ntp.org");

  time_t now = time(nullptr);
  int retry = 0;
  while (now < 100000 && retry < 40)
  {
      Serial.print(".");
      delay(500);
      now = time(nullptr);
      retry++;
  }

  if (now >= 100000) {
      Serial.println("\n✅ Time synchronized successfully!");
  } else {
      Serial.println("\n⚠️ Time synchronization failed (using default ticks).");
  }
}

// ==========================================
// Main Loop
// ==========================================
void loop()
{
  // Maintain MQTT connection
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();

  // Send sensor data at regular intervals
  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis >= INTERVAL) {
    lastMillis = currentMillis;
    
    // Read humidity and temperature
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Validate readings
    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("❌ Error: Failed to read from DHT sensor!");
        return;
    }

    // Read AQI from MQ135
    int ppm = analogRead(MQ135_PIN);
    int aqi = calculateAQI(ppm);

    // Create JSON payload
    String payload = "{";
    payload += "\"temperature\":" + String(temperature, 1) + ",";
    payload += "\"humidity\":" + String(humidity, 0) + ",";
    payload += "\"ppm\":" + String(ppm) + ",";
    payload += "\"aqi\":" + String(aqi) + ",";
    payload += "\"category\":\"" + categoryAQI(aqi) + "\",";
    payload += "\"timestamp\":" + String((int)time(nullptr)) + ",";
    payload += "\"city\":\"" + CITY + "\"";
    payload += "}";

    // Publish to MQTT
    Serial.print("📤 Publishing to MQTT: ");
    Serial.println(payload);
    
    if (mqttClient.publish(MQTT_TOPIC, payload.c_str())) {
      Serial.println("✅ Data published successfully!");
    } else {
      Serial.println("❌ Failed to publish data");
    }

    // Diagnostic output to Serial monitor
    Serial.println("\n-----------------------------");
    Serial.print("Temperature : "); Serial.print(temperature); Serial.println(" *C");
    Serial.print("Humidity    : "); Serial.print(humidity); Serial.println(" %");
    Serial.print("Raw PPM     : "); Serial.println(ppm);
    Serial.print("Calculated AQI : "); Serial.println(aqi);
    Serial.print("Category    : "); Serial.println(categoryAQI(aqi));
    Serial.println("-----------------------------");
  }
}
