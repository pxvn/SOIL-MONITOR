#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include "config.h"
#include "webui.h"

AsyncWebServer server(SERVER_PORT);
AsyncEventSource events("/events");
DHT dht(DHT_PIN, DHT_TYPE);

// Global state
float soilMoisture = 0;
float temperature = 0;
float humidity = 0;
bool pumpActive = false;
bool autoMode = true;
bool sensorError = false;
unsigned long lastMeasurement = 0;
unsigned long lastWiFiCheck = 0;
unsigned long pumpStartTime = 0;
unsigned long lastPumpStop = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\nSoil Monitoring System starting...");
    
    // Initialize hardware
    pinMode(PUMP_RELAY_PIN, OUTPUT);
    digitalWrite(PUMP_RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW); // Ensure pump is off
    delay(100);
    digitalWrite(PUMP_RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW); // Double check
    
    dht.begin();
    
    // Initialize WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Wait for WiFi with timeout
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\nConnected: %s\n", WiFi.localIP().toString().c_str());
        events.send("connected", "wifi", millis());
    } else {
        Serial.println("\nFailed to connect!");
        events.send("disconnected", "wifi", millis());
    }
    
    // Initialize web server
    initWebServer();
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Check WiFi status
    if (currentMillis - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
        static bool lastWiFiStatus = false;
        bool currentWiFiStatus = (WiFi.status() == WL_CONNECTED);
        
        if (currentWiFiStatus != lastWiFiStatus) {
            if (currentWiFiStatus) {
                Serial.println("WiFi Connected");
                events.send("connected", "wifi", millis());
            } else {
                Serial.println("WiFi Disconnected");
                events.send("disconnected", "wifi", millis());
                WiFi.reconnect();
            }
            lastWiFiStatus = currentWiFiStatus;
        }
        lastWiFiCheck = currentMillis;
    }
    
    // Update sensor readings
    if (currentMillis - lastMeasurement >= MEASUREMENT_INTERVAL) {
        updateSensorReadings();
        lastMeasurement = currentMillis;
        
        // Send real-time updates if WiFi is connected
        if (WiFi.status() == WL_CONNECTED) {
            String json = getSensorJson();
            events.send(json.c_str(), "sensors", millis());
        }
    }
    
    // Check pump timeout
    if (pumpActive && (currentMillis - pumpStartTime >= PUMP_TIMEOUT)) {
        stopPump();
    }
    
    // Auto mode control
    if (autoMode && !pumpActive && !sensorError) {
        if (soilMoisture < MOISTURE_THRESHOLD_LOW && 
            (currentMillis - lastPumpStop >= PUMP_COOLDOWN)) {
            startPump();
        }
    }
    
    yield(); // Allow ESP8266 to handle system tasks
}

void initWebServer() {
    // Setup SSE
    events.onConnect([](AsyncEventSourceClient *client) {
        client->send("hello", NULL, millis(), 1000);
    });
    server.addHandler(&events);
    
    // Serve web interface
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", INDEX_HTML);
    });
    
    // API endpoints
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", getSensorJson());
    });
    
    server.on("/api/control", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("auto", true)) {
            autoMode = (request->getParam("auto", true)->value() == "true");
            request->send(200);
            events.send(autoMode ? "auto_on" : "auto_off", "auto", millis());
        }
        else if (request->hasParam("pump", true)) {
            bool shouldPump = (request->getParam("pump", true)->value() == "true");
            if (shouldPump && !pumpActive && !sensorError) {
                startPump();
            } else if (!shouldPump && pumpActive) {
                stopPump();
            }
            request->send(200);
        }
        else {
            request->send(400);
        }
    });
    
    server.begin();
    Serial.println("Web server started");
}

void updateSensorReadings() {
    // Read soil moisture with proper voltage handling
    int rawValue = 0;
    for (int i = 0; i < 5; i++) {
        rawValue += analogRead(SOIL_MOISTURE_PIN);
        delay(10);
    }
    rawValue /= 5; // Average of 5 readings
    
    // Convert ADC reading to voltage (NodeMCU ADC reads 0-1V)
    float voltage = (rawValue * VOLTAGE_MAX) / ADC_MAX;
    
    // Calculate percentage (inverted as higher voltage means drier soil)
    // Assuming linear relationship between voltage and moisture
    float percentage = (1.0 - (voltage / VOLTAGE_MAX)) * 100.0;
    
    // Constrain to valid range
    soilMoisture = constrain(percentage, 0, 100);
    
    // Read temperature and humidity
    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();
    
    if (!isnan(newTemp) && !isnan(newHum)) {
        temperature = newTemp;
        humidity = newHum;
        sensorError = false;
    } else {
        sensorError = true;
        Serial.println("DHT sensor error");
    }
    
    // Debug output
    Serial.printf("Raw ADC: %d, Voltage: %.2fV\n", rawValue, voltage);
    Serial.printf("Moisture: %.1f%%, Temp: %.1f°C, Humidity: %.1f%%\n", 
                 soilMoisture, temperature, humidity);
}

String getSensorJson() {
    String json = "{";
    json += "\"soil_moisture\":" + String(soilMoisture, 1) + ",";
    json += "\"temperature\":" + String(temperature, 1) + ",";
    json += "\"humidity\":" + String(humidity, 1) + ",";
    json += "\"pump_active\":" + String(pumpActive ? "true" : "false") + ",";
    json += "\"auto_mode\":" + String(autoMode ? "true" : "false") + ",";
    json += "\"sensor_error\":" + String(sensorError ? "true" : "false");
    json += "}";
    return json;
}

void startPump() {
    digitalWrite(PUMP_RELAY_PIN, RELAY_ACTIVE_LOW ? LOW : HIGH);
    pumpActive = true;
    pumpStartTime = millis();
    Serial.println("Pump started");
    events.send("pump_on", "pump", millis());
}

void stopPump() {
    digitalWrite(PUMP_RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
    pumpActive = false;
    lastPumpStop = millis();
    Serial.println("Pump stopped");
    events.send("pump_off", "pump", millis());
} 