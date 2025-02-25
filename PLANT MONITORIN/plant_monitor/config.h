#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "0000"
#define WIFI_PASSWORD "12121212"

#define SERVER_PORT 80

#define DHT_PIN D4
#define DHT_TYPE DHT22
#define SOIL_MOISTURE_PIN A0
#define WATER_LEVEL_PIN D5
#define PUMP_RELAY_PIN D1

#define MEASUREMENT_INTERVAL 5000  // 5 seconds
#define PUMP_TIMEOUT 10000        // 10 seconds
#define MOISTURE_THRESHOLD_LOW 30  // 30%
#define MOISTURE_THRESHOLD_HIGH 70 // 70%

// EEPROM Configuration
#define EEPROM_SIZE 512
#define CONFIG_START_ADDRESS 0

#endif 