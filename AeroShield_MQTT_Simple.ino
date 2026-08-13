/*
  AeroShield Air Quality & Climate Monitor - Single File Version
  Device: NodeMCU (ESP8266)
  Sensors: DHT11 (Temperature/Humidity) & MQ135 (Air Quality/PPM)
  Cloud Integration: HiveMQ MQTT Broker
  
  PIN CONNECTIONS:
  - DHT11 Data Pin: D4
  - MQ135 Analog Pin: A0
  
  ACCESS POINT SETUP:
  - Connect to WiFi: "AeroShield-Setup"
  - Password: "aeroshield123"
  - Configure: WiFi credentials, City name, MQTT username & password
  
  MQTT CREDENTIALS:
  - Broker: 97f6bb83bebf454c86a437210b5379b9.s1.eu.hivemq.cloud
  - Port: 8883
  - Default Username: anand
  - Default Password: anand@1234
  - Topic: aeroshield/sensors/data
  
  INSTALL LIBRARIES (Arduino Library Manager):
  1. "DHT sensor library" by Adafruit
  2. "Adafruit Unified Sensor"
  3. "PubSubClient" by Nick O'Leary
  4. "WiFiManager" by tzapu
  
  BOARD: NodeMCU 1.0 (ESP-12E Module)
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
// Hardware Pins
// ==========================================
#define DHTPIN D4         // DHT11 Data Pin
#define DHTTYPE DHT11     // DHT sensor type
#define MQ135_PIN A0      // MQ135 Analog Pin

// Initialize objects
DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);
WiFiManager wifiManager;

// Variables
String CITY = "chikodi";
String MQTT_USER = "anand";
String MQTT_PASS = "anand@1234";
unsigned long lastMillis = 0;
const long INTERVAL = 10000; // Send data every 10 seconds

// ==========================================
// Helper Functions
// ==========================================
String categoryAQI(int aqi) {
  if (aqi <= 50) return "Good";
  if (aqi <= 100) return "Moderate";
  if (aqi <= 150) return "USG";
  if (aqi <= 200) return "Unhealthy";
  if (aqi <= 300) return "Very Unhealthy";
  return "Hazardous";
}

int calculateAQI(int ppm) {
  int aqi = map(ppm, 0, 1023, 0, 500);
  if (aqi < 0) aqi = 0;
  if (aqi > 500) aqi = 500;
  return aqi;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void connectMQTT() {
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
void setup() {
  Serial.begin(9600);
  delay(10);
  
  Serial.println("\n--- AeroShield Air Quality Monitor Startup ---");
  
  // Initialize Sensor
  dht.begin();
  
  // WiFiManager Configuration
  wifiManager.setAPName("AeroShield-Setup");
  wifiManager.setAPPassword("aeroshield123");
  
  // Add custom parameters for city and MQTT credentials
  WiFiManagerParameter customCity("city", "City Name", CITY.c_str(), 20);
  WiFiManagerParameter customMqttUser("mqtt_user", "MQTT Username", MQTT_USER.c_str(), 30);
  WiFiManagerParameter customMqttPass("mqtt_pass", "MQTT Password", MQTT_PASS.c_str(), 30);
  
  wifiManager.addParameter(&customCity);
  wifiManager.addParameter(&customMqttUser);
  wifiManager.addParameter(&customMqttPass);
  
  // Connect to WiFi or start AP
  if (!wifiManager.autoConnect("AeroShield-Setup", "aeroshield123")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  Serial.println("✅ WiFi Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  // Get values from custom parameters
  CITY = customCity.getValue();
  MQTT_USER = customMqttUser.getValue();
  MQTT_PASS = customMqttPass.getValue();
  
  Serial.print("City: ");
  Serial.println(CITY);
  Serial.print("MQTT Username: ");
  Serial.println(MQTT_USER);
  
  // Connect to MQTT
  connectMQTT();

  // Sync time
  Serial.print("Syncing time...");
  configTime(19800, 0, "pool.ntp.org"); // IST UTC+5.5

  time_t now = time(nullptr);
  int retry = 0;
  while (now < 100000 && retry < 40) {
    Serial.print(".");
    delay(500);
    now = time(nullptr);
    retry++;
  }

  if (now >= 100000) {
    Serial.println("\n✅ Time synced!");
  } else {
    Serial.println("\n⚠️ Time sync failed");
  }
}

// ==========================================
// Main Loop
// ==========================================
void loop() {
  // Maintain MQTT connection
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();

  // Send data at intervals
  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis >= INTERVAL) {
    lastMillis = currentMillis;
    
    // Read sensors
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("❌ DHT sensor error!");
      return;
    }

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
    Serial.print("📤 Publishing: ");
    Serial.println(payload);
    
    if (mqttClient.publish(MQTT_TOPIC, payload.c_str())) {
      Serial.println("✅ Published!");
    } else {
      Serial.println("❌ Publish failed");
    }

    // Debug output
    Serial.println("----------------------------");
    Serial.print("Temp: "); Serial.print(temperature); Serial.println("°C");
    Serial.print("Humidity: "); Serial.print(humidity); Serial.println("%");
    Serial.print("PPM: "); Serial.println(ppm);
    Serial.print("AQI: "); Serial.println(aqi);
    Serial.print("Category: "); Serial.println(categoryAQI(aqi));
    Serial.println("----------------------------");
  }
}
