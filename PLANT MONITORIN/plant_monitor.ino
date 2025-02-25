#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

// Pin Definitions
#define SOIL_MOISTURE_PIN A0    // Soil moisture sensor analog pin
#define DHT_PIN D4             // DHT11 data pin
#define PUMP_RELAY_PIN D1      // Water pump relay control pin
#define DHT_TYPE DHT11         // DHT sensor type

// Network Configuration
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Constants
const int MOISTURE_THRESHOLD_LOW = 30;   // Low moisture threshold (%)
const int MOISTURE_THRESHOLD_HIGH = 70;  // High moisture threshold (%)
const unsigned long PUMP_TIMEOUT = 10000; // Maximum pump run time (ms)
const unsigned long MEASUREMENT_INTERVAL = 60000; // Sensor reading interval (ms)

// Global Variables
ESP8266WebServer server(80);
DHT dht(DHT_PIN, DHT_TYPE);

struct SystemState {
  float soilMoisture;
  float temperature;
  float humidity;
  bool pumpActive;
  bool autoMode;
  unsigned long lastWatering;
  unsigned long lastMeasurement;
} state;

// System Configuration (stored in EEPROM)
struct Config {
  bool autoMode;
  int moistureThresholdLow;
  int moistureThresholdHigh;
} config;

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  digitalWrite(PUMP_RELAY_PIN, LOW);
  
  // Initialize sensors
  dht.begin();
  
  // Initialize EEPROM and load config
  EEPROM.begin(512);
  loadConfig();
  
  // Initialize WiFi
  setupWiFi();
  
  // Setup web server routes
  setupWebServer();
  
  // Initialize state
  state.pumpActive = false;
  state.lastWatering = 0;
  state.lastMeasurement = 0;
  state.autoMode = config.autoMode;
}

void loop() {
  server.handleClient();
  
  unsigned long currentMillis = millis();
  
  // Regular sensor measurements
  if (currentMillis - state.lastMeasurement >= MEASUREMENT_INTERVAL) {
    updateSensorReadings();
    state.lastMeasurement = currentMillis;
  }
  
  // Automated watering control
  if (state.autoMode) {
    handleAutomatedWatering();
  }
  
  // Safety check for pump timeout
  if (state.pumpActive && (currentMillis - state.lastWatering >= PUMP_TIMEOUT)) {
    stopPump();
  }
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupWebServer() {
  // API endpoints
  server.on("/api/status", HTTP_GET, handleGetStatus);
  server.on("/api/control", HTTP_POST, handleControl);
  server.on("/api/config", HTTP_POST, handleConfig);
  
  // Serve static files
  server.serveStatic("/", SPIFFS, "/index.html");
  
  server.begin();
}

void updateSensorReadings() {
  // Read soil moisture
  int rawMoisture = analogRead(SOIL_MOISTURE_PIN);
  state.soilMoisture = map(rawMoisture, 1023, 0, 0, 100); // Adjust mapping based on sensor calibration
  
  // Read temperature and humidity
  state.temperature = dht.readTemperature();
  state.humidity = dht.readHumidity();
  
  // Check for sensor errors
  if (isnan(state.temperature) || isnan(state.humidity)) {
    Serial.println("Failed to read from DHT sensor!");
  }
}

void handleAutomatedWatering() {
  if (state.soilMoisture < config.moistureThresholdLow && !state.pumpActive) {
    startPump();
  } else if (state.soilMoisture > config.moistureThresholdHigh && state.pumpActive) {
    stopPump();
  }
}

void startPump() {
  digitalWrite(PUMP_RELAY_PIN, HIGH);
  state.pumpActive = true;
  state.lastWatering = millis();
}

void stopPump() {
  digitalWrite(PUMP_RELAY_PIN, LOW);
  state.pumpActive = false;
}

void handleGetStatus() {
  StaticJsonDocument<200> doc;
  
  doc["soil_moisture"] = state.soilMoisture;
  doc["temperature"] = state.temperature;
  doc["humidity"] = state.humidity;
  doc["pump_active"] = state.pumpActive;
  doc["auto_mode"] = state.autoMode;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleControl() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (!error) {
      if (doc.containsKey("pump")) {
        bool shouldActivate = doc["pump"];
        if (shouldActivate && !state.pumpActive) {
          startPump();
        } else if (!shouldActivate && state.pumpActive) {
          stopPump();
        }
      }
      
      if (doc.containsKey("auto_mode")) {
        state.autoMode = doc["auto_mode"];
        config.autoMode = state.autoMode;
        saveConfig();
      }
      
      server.send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    }
  } else {
    server.send(400, "application/json", "{\"error\":\"No data received\"}");
  }
}

void handleConfig() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (!error) {
      if (doc.containsKey("moisture_low")) {
        config.moistureThresholdLow = doc["moisture_low"];
      }
      if (doc.containsKey("moisture_high")) {
        config.moistureThresholdHigh = doc["moisture_high"];
      }
      saveConfig();
      server.send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    }
  } else {
    server.send(400, "application/json", "{\"error\":\"No data received\"}");
  }
}

void loadConfig() {
  EEPROM.get(0, config);
  // Set defaults if EEPROM is empty
  if (isnan(config.moistureThresholdLow) || isnan(config.moistureThresholdHigh)) {
    config.autoMode = true;
    config.moistureThresholdLow = MOISTURE_THRESHOLD_LOW;
    config.moistureThresholdHigh = MOISTURE_THRESHOLD_HIGH;
    saveConfig();
  }
}

void saveConfig() {
  EEPROM.put(0, config);
  EEPROM.commit();
} 