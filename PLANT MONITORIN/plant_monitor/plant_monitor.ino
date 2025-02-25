#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <EEPROM.h>
#include "config.h"
#include "types.h"
#include "webui.h"

AsyncWebServer server(SERVER_PORT);
AsyncEventSource events("/events");  // Server-sent events
DHT dht(DHT_PIN, DHT_TYPE);
SystemState state;
SystemConfig config;
unsigned long lastWiFiCheck = 0;
bool lastWiFiStatus = false;

void setup() {
    Serial.begin(115200);
    initSystem();
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Check WiFi status every second
    if (currentMillis - lastWiFiCheck >= 1000) {
        bool currentWiFiStatus = WiFi.status() == WL_CONNECTED;
        if (currentWiFiStatus != lastWiFiStatus) {
            lastWiFiStatus = currentWiFiStatus;
            if (currentWiFiStatus) {
                events.send("connected", "wifi", millis());
            } else {
                events.send("disconnected", "wifi", millis());
                // Try to reconnect
                WiFi.reconnect();
            }
        }
        lastWiFiCheck = currentMillis;
    }
    
    // Update sensor readings
    if (currentMillis - state.lastMeasurement >= MEASUREMENT_INTERVAL) {
        updateSensorReadings();
        state.lastMeasurement = currentMillis;
        
        // Send real-time updates via SSE
        if (lastWiFiStatus) {
            StaticJsonDocument<200> doc;
            doc["soil_moisture"] = state.soilMoisture;
            doc["temperature"] = state.temperature;
            doc["humidity"] = state.humidity;
            doc["water_level"] = analogRead(WATER_LEVEL_PIN);
            doc["pump_active"] = state.pumpActive;
            doc["auto_mode"] = state.autoMode;
            
            String data;
            serializeJson(doc, data);
            events.send(data.c_str(), "sensors", millis());
        }
    }
    
    // Automated watering control
    if (state.autoMode && state.soilMoisture < config.moistureThresholdLow && !state.pumpActive) {
        startPump();
        events.send("pump_on", "pump", millis());
    } else if ((state.autoMode && state.soilMoisture > config.moistureThresholdHigh) || 
              (state.pumpActive && currentMillis - state.lastWatering >= PUMP_TIMEOUT)) {
        stopPump();
        events.send("pump_off", "pump", millis());
    }
    
    // Handle OTA updates and other background tasks
    yield();
}

void initSystem() {
    // Initialize hardware
    pinMode(PUMP_RELAY_PIN, OUTPUT);
    digitalWrite(PUMP_RELAY_PIN, LOW);
    dht.begin();
    
    // Initialize state
    state = {0, 0, 0, false, true, 0, 0};
    
    // Initialize EEPROM and load config
    EEPROM.begin(EEPROM_SIZE);
    loadConfig();
    
    // Initialize WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Wait for initial connection
    int connectionAttempts = 0;
    while (WiFi.status() != WL_CONNECTED && connectionAttempts < 20) {
        delay(500);
        Serial.print(".");
        connectionAttempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\nConnected: %s\n", WiFi.localIP().toString().c_str());
        lastWiFiStatus = true;
    } else {
        Serial.println("\nFailed to connect. Will retry in background.");
        lastWiFiStatus = false;
    }
    
    // Initialize web server
    initWebServer();
    
    // Initial sensor reading
    updateSensorReadings();
}

void initWebServer() {
    // Initialize Server-Sent Events
    events.onConnect([](AsyncEventSourceClient *client) {
        if (client->lastId()) {
            Serial.printf("Client reconnected! Last message ID: %u\n", client->lastId());
        }
        client->send("hello", NULL, millis(), 1000);
    });
    server.addHandler(&events);
    
    // Serve static files
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", INDEX_HTML);
    });
    
    // API endpoints
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        StaticJsonDocument<200> doc;
        
        doc["soil_moisture"] = state.soilMoisture;
        doc["temperature"] = state.temperature;
        doc["humidity"] = state.humidity;
        doc["water_level"] = analogRead(WATER_LEVEL_PIN);
        doc["pump_active"] = state.pumpActive;
        doc["auto_mode"] = state.autoMode;
        doc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
        doc["signal_strength"] = WiFi.RSSI();
        
        serializeJson(doc, *response);
        request->send(response);
    });
    
    AsyncCallbackJsonWebHandler *controlHandler = new AsyncCallbackJsonWebHandler("/api/control", [](AsyncWebServerRequest *request, JsonVariant &json) {
        StaticJsonDocument<200> doc = json.as<JsonDocument>();
        
        if (doc.containsKey("pump")) {
            bool shouldActivate = doc["pump"];
            if (shouldActivate && !state.pumpActive) {
                startPump();
                events.send("pump_on", "pump", millis());
            } else if (!shouldActivate && state.pumpActive) {
                stopPump();
                events.send("pump_off", "pump", millis());
            }
        }
        
        if (doc.containsKey("auto_mode")) {
            state.autoMode = doc["auto_mode"];
            config.autoMode = state.autoMode;
            saveConfig();
            events.send(state.autoMode ? "auto_on" : "auto_off", "auto", millis());
        }
        
        request->send(200, "application/json", "{\"status\":\"success\"}");
    });
    
    server.addHandler(controlHandler);
    server.begin();
}

void updateSensorReadings() {
    // Read soil moisture with error checking
    int rawMoisture = analogRead(SOIL_MOISTURE_PIN);
    if (rawMoisture != 0 && rawMoisture != 1023) {  // Basic error checking
        state.soilMoisture = map(rawMoisture, 1023, 0, 0, 100);
    }
    
    // Read temperature and humidity with validation
    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();
    
    if (!isnan(newTemp) && !isnan(newHum)) {
        state.temperature = newTemp;
        state.humidity = newHum;
    }
    
    Serial.printf("Moisture: %.1f%%, Temp: %.1f°C, Humidity: %.1f%%\n", 
                 state.soilMoisture, state.temperature, state.humidity);
}

void startPump() {
    digitalWrite(PUMP_RELAY_PIN, HIGH);
    state.pumpActive = true;
    state.lastWatering = millis();
    Serial.println("Pump started");
}

void stopPump() {
    digitalWrite(PUMP_RELAY_PIN, LOW);
    state.pumpActive = false;
    Serial.println("Pump stopped");
}

void loadConfig() {
    EEPROM.get(CONFIG_START_ADDRESS, config);
    
    if (isnan(config.moistureThresholdLow) || isnan(config.moistureThresholdHigh)) {
        config = {true, MOISTURE_THRESHOLD_LOW, MOISTURE_THRESHOLD_HIGH};
        saveConfig();
    }
    
    state.autoMode = config.autoMode;
}

void saveConfig() {
    EEPROM.put(CONFIG_START_ADDRESS, config);
    EEPROM.commit();
} 