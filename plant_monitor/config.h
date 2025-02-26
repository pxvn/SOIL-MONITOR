#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "0000"
#define WIFI_PASSWORD "12121212"

// Server Configuration
#define SERVER_PORT 80

// Pin Definitions
#define DHT_PIN D4          // GPIO2
#define DHT_TYPE DHT11      // Using DHT11
#define SOIL_MOISTURE_PIN A0 // Analog pin
#define PUMP_RELAY_PIN D1   // GPIO5

// NodeMCU ADC Calibration (0-1V input)
// These values need to be adjusted based on your sensor's voltage output
// For 3.3V sensor through voltage divider
#define ADC_MAX 1023        // 10-bit ADC
#define VOLTAGE_MAX 1.0     // NodeMCU can only read up to 1V
#define SENSOR_VOLTAGE 3.3  // Original sensor voltage

// Soil Moisture Calibration
// These are percentage thresholds
#define MOISTURE_THRESHOLD_LOW 30    // 30% threshold for dry soil
#define MOISTURE_THRESHOLD_HIGH 70   // 70% threshold for wet soil

// System Parameters
#define MEASUREMENT_INTERVAL 1000    // 1 second between readings
#define PUMP_TIMEOUT 10000          // 10 seconds max pump runtime
#define PUMP_COOLDOWN 5000         // 5 seconds cooldown
#define RELAY_ACTIVE_LOW true      // Set to true if relay triggers on LOW
#define WIFI_CHECK_INTERVAL 1000   // Check WiFi every second

// Memory Optimization
#define JSON_BUFFER_SIZE 200
#define MAX_SENSOR_ERRORS 3

// EEPROM Configuration
#define EEPROM_SIZE 512
#define CONFIG_START_ADDRESS 0

#endif 