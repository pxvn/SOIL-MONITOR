# Smart Agriculture Monitoring System

An IoT-based agriculture monitoring and control system using ESP8266. Monitor soil conditions and automate irrigation in real-time through a modern web interface.

## System Features

• Real-time Monitoring:
  - Soil Moisture (0-100%)
  - Temperature (-40°C to 80°C)
  - Humidity (0-100%)
  - Connection Status (Live)
  - Sensor Error Detection

• Automated Control:
  - Smart Water Pump Control
  - 5-second Safety Timer
  - Auto-shutdown Protection
  - Error Prevention
  - Touch-friendly Interface

• Smart Connectivity:
  - WiFi Access Point (SOIL_MONITOR)
  - No Internet Required
  - Auto-reconnection
  - WebSocket Communication
  - Real-time Updates (1s)

• User Interface:
  - Mobile-optimized Design
  - Card-based Layout
  - Error Indicators
  - Status Notifications
  - Touch Controls

## Hardware Setup

**Components Required:**
- ESP8266 NodeMCU Board
- DHT11 Temperature & Humidity Sensor
- Capacitive Soil Moisture Sensor
- 5V Relay Module
- DC Water Pump (5-12V)
- Power Supply Unit
- Connecting Wires

**Pin Connections:**
```
Component         ESP8266 Pin    Description
---------        -----------    -----------
DHT11            D4 (GPIO2)     Data Pin
Soil Sensor      A0             Analog Input
Relay/Pump       D1 (GPIO5)     Digital Out
Status LED       D2 (GPIO4)     Indicator
```

## Quick Installation

1. **Arduino IDE Setup:**
   ```
   File -> Preferences -> Additional Board Manager URLs:
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```

2. **Required Libraries:**
   - ESP8266WiFi
   - ESPAsyncWebServer
   - DHT sensor library

3. **Upload Process:**
   - Open `soil.ino`
   - Select "NodeMCU 1.0" board
   - Set Upload Speed: 115200
   - Upload the code

4. **First Run:**
   - Power on the system
   - Connect to "SOIL_MONITOR" WiFi
   - Password: "12345678"
   - Access dashboard: http://192.168.4.1

## Operation Guide

1. **Monitoring:**
   - All sensors update every second
   - Red indicators show sensor errors
   - Connection status at top
   - Values show -- when disconnected

2. **Pump Control:**
   - Tap switch to start pump
   - 5-second auto-shutoff
   - Timer shows countdown
   - Auto-stops if errors detected

3. **Maintenance:**
   - Check sensor connections if errors persist
   - Keep soil sensor clean
   - Verify pump operation regularly
   - Monitor water level in tank

## Safety Features

- Pump timeout protection
- Sensor error detection
- Connection loss handling
- Auto-reconnection
- Error state indicators

## Author & Version
- Created by: Pavan Kalsariya