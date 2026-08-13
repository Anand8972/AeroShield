/*
  Air Quality & Climate Monitor
  Device: NodeMCU (ESP8266)
  Sensors: DHT11 (Temperature/Humidity) & MQ135 (Air Quality/PPM)
  Cloud Integration: Firebase Realtime Database
  
  Dependencies (Install via Arduino Library Manager):
  1. "DHT sensor library" by Adafruit
  2. "Adafruit Unified Sensor" (automatically prompted by DHT library)
  3. "Firebase ESP Client" by Mobizt (Specifically: Firebase_ESP_Client.h)
  
  Board Manager URL (in Preferences):
  http://arduino.esp8266.com/stable/package_esp8266com_index.json
  Board Select: "NodeMCU 1.0 (ESP-12E Module)"
*/

#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>
#include <time.h>

// Firebase Helper Libraries (must be included after Firebase_ESP_Client.h)
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// ==========================================
// WiFi Credentials
// ==========================================
#define WIFI_SSID "Anand"
#define WIFI_PASSWORD "anand8972"

// ==========================================
// Firebase Credentials
// ==========================================
#define API_KEY "AIzaSyDX1gHLawkWsztbiMEcB7JfiYvJe2oWtxY"
#define DATABASE_URL "https://airqualitymonitor-e1b95-7c27e-default-rtdb.asia-southeast1.firebasedatabase.app/"

// ==========================================
// Firebase Authentication Credentials
// ==========================================
#define USER_EMAIL "anandsanabe84@gmail.com"
#define USER_PASSWORD "anand@/1234"

// ==========================================
// Hardware Pins Definition
// ==========================================
#define DHTPIN D4         // DHT11 Data Pin connected to D4
#define DHTTYPE DHT11     // DHT sensor type (DHT11)
#define MQ135_PIN A0      // MQ135 Analog Pin connected to A0

// Initialize DHT Sensor
DHT dht(DHTPIN, DHTTYPE);

// Firebase Core Objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Configuration
String CITY = "chikodi";

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
// WiFi Connection Routine
// ==========================================
void connectWiFi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi (");
  Serial.print(WIFI_SSID);
  Serial.print(")");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts > 60) { // Timeout after 30 seconds
      Serial.println("\nWiFi connection failed! Retrying...");
      attempts = 0;
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
  }

  Serial.println("\nWiFi Connected successfully!");
  Serial.print("Local IP Address: ");
  Serial.println(WiFi.localIP());
}

// ==========================================
// Firebase Connection & Authorization Routine
// ==========================================
void connectFirebase()
{
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign token status callback
  config.token_status_callback = tokenStatusCallback;

  // Initialize Firebase client
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.print("Waiting for Firebase Connection");
  unsigned long timeout = millis();

  while (!Firebase.ready())
  {
    Serial.print(".");
    if (millis() - timeout > 30000) // Timeout after 30 seconds
    {
      Serial.println("\nFirebase Authentication Timeout! Check credentials and network rules.");
      return;
    }
    delay(500);
  }

  Serial.println("\nFirebase Connected Successfully!");
}

// ==========================================
// Setup
// ==========================================
void setup()
{
  Serial.begin(9600);
  delay(10);
  
  Serial.println("\n--- Air Quality Monitor Startup ---");
  
  // Initialize Sensor
  dht.begin();
  
  // Connect to Network
  connectWiFi();
  
  // Connect to Firebase
  connectFirebase();

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
      Serial.println("\nTime synchronized successfully!");
  } else {
      Serial.println("\nTime synchronization failed (using default ticks).");
  }
}

// ==========================================
// Upload Live Real-time Status
// ==========================================
void uploadRealtime(float t, float h, int ppm, int aqi)
{
  String path = "/cities/" + CITY + "/realtime";

  FirebaseJson json;
  json.set("temperature", t);
  json.set("humidity", h);
  json.set("ppm", ppm);
  json.set("aqi", aqi);
  json.set("category", categoryAQI(aqi));
  json.set("timestamp", (int)time(nullptr));

  Serial.print("Uploading real-time data to " + path + "... ");
  if (Firebase.RTDB.setJSON(&fbdo, path, &json))
  {
      Serial.println("Success!");
  }
  else
  {
      Serial.print("Failed! Reason: ");
      Serial.println(fbdo.errorReason());
  }
}

// ==========================================
// Upload Historical Record
// ==========================================
void uploadHistory(float t, float h, int ppm, int aqi)
{
  int ts = (int)time(nullptr);
  String path = "/cities/" + CITY + "/history/" + String(ts);

  FirebaseJson json;
  json.set("temperature", t);
  json.set("humidity", h);
  json.set("ppm", ppm);
  json.set("aqi", aqi);
  json.set("category", categoryAQI(aqi));
  json.set("timestamp", ts);

  Serial.print("Uploading historical data to " + path + "... ");
  if (Firebase.RTDB.setJSON(&fbdo, path, &json))
  {
      Serial.println("Success!");
  }
  else
  {
      Serial.print("Failed! Reason: ");
      Serial.println(fbdo.errorReason());
  }
}

// ==========================================
// Main Loop
// ==========================================
void loop()
{
  // Read humidity and temperature
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Validate readings
  if (isnan(temperature) || isnan(humidity))
  {
      Serial.println("Error: Failed to read from DHT sensor! Retrying in 2 seconds...");
      delay(2000);
      return;
  }

  // Read AQI from MQ135
  int ppm = analogRead(MQ135_PIN);
  int aqi = calculateAQI(ppm);

  // Diagnostic output to Serial monitor
  Serial.println("\n-----------------------------");
  Serial.print("Temperature : "); Serial.print(temperature); Serial.println(" *C");
  Serial.print("Humidity    : "); Serial.print(humidity); Serial.println(" %");
  Serial.print("Raw PPM     : "); Serial.println(ppm);
  Serial.print("Calculated AQI : "); Serial.println(aqi);
  Serial.print("Category    : "); Serial.println(categoryAQI(aqi));
  Serial.println("-----------------------------");

  // Push to Firebase if ready
  if (Firebase.ready())
  {
      uploadRealtime(temperature, humidity, ppm, aqi);
      uploadHistory(temperature, humidity, ppm, aqi);
  }
  else
  {
      Serial.println("Firebase warning: Connection is not ready. Reconnecting...");
  }

  // Sample and push every 10 seconds (adjust as required)
  delay(10000);
}
