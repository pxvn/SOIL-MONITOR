/*
 * Agriculture Monitoring System
 * Author: Pavan Kalsariya
 * Date: 2024
 * Version: 1.0
 * Hardware: ESP8266 NodeMCU
 * Description: Real-time soil moisture, temperature, and humidity monitoring system
 * with web interface and pump control
 */
  
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include "webui.h"

// WiFi credentials for AP mode
const char* AP_SSID = "SOIL_MONITOR";  // AP mode SSID
const char* AP_PASSWORD = "12345678";   // AP mode password

// DNS Server for captive portal
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);

// Pin Definitions
#define SOIL_MOISTURE_PIN A0    // Soil moisture sensor analog pin
#define DHT_PIN D4             // DHT11 data pin
#define RELAY_PIN D1           // Relay control pin
#define DHT_TYPE DHT11         // DHT sensor type
#define LED_PIN LED_BUILTIN    // Built-in LED for status indication

// Soil moisture calibration values
const int WET_VALUE = 820;    // Sensor value when soil is wet
const int DRY_VALUE = 1024;   // Sensor value when soil is dry

// Sensor error thresholds
const int MOISTURE_ERROR_THRESHOLD = 5;    // Consider sensor error if reading varies too quickly
const float TEMP_MIN_VALID = 0.0;         // Minimum valid temperature
const float TEMP_MAX_VALID = 50.0;        // Maximum valid temperature
const float HUMIDITY_MIN_VALID = 0.0;     // Minimum valid humidity
const float HUMIDITY_MAX_VALID = 100.0;   // Maximum valid humidity

// Pump safety timeout
const unsigned long PUMP_TIMEOUT = 5000;  // 5 seconds max pump runtime
unsigned long pumpStartTime = 0;

// Global variables
DHT dht(DHT_PIN, DHT_TYPE);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
DNSServer dnsServer;
bool pumpState = false;
unsigned long lastUpdate = 0;
const long updateInterval = 2000;  // Update interval in milliseconds

// Connection status LED
unsigned long lastLedBlink = 0;
const long LED_BLINK_INTERVAL = 2000;  // LED blink interval when client connected
bool clientConnected = false;

// Sensor state variables
float lastMoisture = -1;
bool moistureError = false;
bool temperatureError = false;
bool humidityError = false;

class CaptiveRequestHandler : public AsyncWebHandler {
public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
        // Handle all requests
        return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
        // Android requires this header for captive portal detection
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html);
        response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        response->addHeader("Pragma", "no-cache");
        response->addHeader("Expires", "-1");
        
        // Add Android captive portal detection response
        if (request->host() != apIP.toString()) {
            response->addHeader("Location", String("http://") + apIP.toString());
        }
        
        request->send(response);
    }
};

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    Serial.println("\nInitializing Agriculture Monitoring System...");

    // Initialize pins
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);  // Start with pump off
    digitalWrite(LED_PIN, HIGH);   // LED off initially

    // Initialize DHT sensor
    dht.begin();
    delay(2000);  // Give time for DHT sensor to stabilize

    // Setup AP mode with captive portal
    setupAP();

    // Initialize WebSocket
    ws.onEvent(onEvent);
    server.addHandler(&ws);

    // Add captive portal handler
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);

    // Start server
    server.begin();
    Serial.println("HTTP server started");
    digitalWrite(LED_PIN, LOW);  // Turn LED on when everything is ready
}

void setupAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    // Configure DNS server to redirect all domains to our IP
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", apIP);

    Serial.println("\nAP Mode configured");
    Serial.print("SSID: ");
    Serial.println(AP_SSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
}

void loop() {
    dnsServer.processNextRequest();
    ws.cleanupClients();
    
    // Handle LED status indicator
    if (clientConnected && (millis() - lastLedBlink >= LED_BLINK_INTERVAL)) {
        digitalWrite(LED_PIN, HIGH);  // LED off
        delay(100);
        digitalWrite(LED_PIN, LOW);   // LED on
        lastLedBlink = millis();
    }
    
    // Check pump timeout
    if (pumpState && (millis() - pumpStartTime >= PUMP_TIMEOUT)) {
        pumpState = false;
        digitalWrite(RELAY_PIN, LOW);
        sendSensorData();  // Update clients about pump state change
    }

    // Send sensor data every updateInterval
    if ((millis() - lastUpdate) > updateInterval) {
        checkSensors();
        sendSensorData();
        lastUpdate = millis();
    }
}

void checkSensors() {
    // Check DHT sensor
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    
    temperatureError = isnan(temperature) || temperature < TEMP_MIN_VALID || temperature > TEMP_MAX_VALID;
    humidityError = isnan(humidity) || humidity < HUMIDITY_MIN_VALID || humidity > HUMIDITY_MAX_VALID;
    
    // Check soil moisture sensor
    int currentMoisture = getMoisturePercentage();
    if (lastMoisture >= 0) {
        // Check for unrealistic changes in moisture
        moistureError = abs(currentMoisture - lastMoisture) > MOISTURE_ERROR_THRESHOLD;
    }
    lastMoisture = currentMoisture;
    
    // Log sensor errors
    if (temperatureError || humidityError || moistureError) {
        Serial.println("Sensor Errors Detected:");
        if (temperatureError) Serial.println("- Temperature sensor error");
        if (humidityError) Serial.println("- Humidity sensor error");
        if (moistureError) Serial.println("- Moisture sensor error");
    }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        
        // Parse JSON command
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);
        
        if (!error) {
            if (doc.containsKey("command") && doc["command"] == "pump") {
                bool newPumpState = doc["state"] == 1;
                if (newPumpState != pumpState) {
                    pumpState = newPumpState;
                    digitalWrite(RELAY_PIN, pumpState ? HIGH : LOW);
                    
                    if (pumpState) {
                        pumpStartTime = millis();  // Start pump timer
                    }
                    
                    sendSensorData();  // Send immediate update
                }
            }
        }
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
            void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            clientConnected = true;
            sendSensorData();  // Send data immediately on connect
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            clientConnected = (ws.count() > 0);  // Check if any clients are still connected
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

int getMoisturePercentage() {
    int rawValue = analogRead(SOIL_MOISTURE_PIN);
    int moisturePercentage = map(rawValue, DRY_VALUE, WET_VALUE, 0, 100);
    return constrain(moisturePercentage, 0, 100);
}

void sendSensorData() {
    StaticJsonDocument<200> doc;
    
    // Read sensor values
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    int moisture = getMoisturePercentage();
    
    // Add sensor values and error states to JSON
    doc["temperature"] = temperatureError ? 0 : round(temperature * 10.0) / 10.0;
    doc["humidity"] = humidityError ? 0 : round(humidity * 10.0) / 10.0;
    doc["moisture"] = moistureError ? 0 : moisture;
    doc["pumpState"] = pumpState ? 1 : 0;
    
    // Add error flags
    doc["temperature_error"] = temperatureError;
    doc["humidity_error"] = humidityError;
    doc["moisture_error"] = moistureError;
    
    String output;
    serializeJson(doc, output);
    ws.textAll(output);
    
    // Log values to Serial
    Serial.println("Sensor Readings:");
    Serial.printf("Temperature: %.1f°C (Error: %s)\n", temperature, temperatureError ? "Yes" : "No");
    Serial.printf("Humidity: %.1f%% (Error: %s)\n", humidity, humidityError ? "Yes" : "No");
    Serial.printf("Moisture: %d%% (Error: %s)\n", moisture, moistureError ? "Yes" : "No");
    Serial.printf("Pump State: %s\n", pumpState ? "ON" : "OFF");
    Serial.println("------------------------");
} 