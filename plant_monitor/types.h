#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>  // For uint8_t type

struct SystemState {
    float soilMoisture;
    float temperature;
    float humidity;
    bool pumpActive;
    bool autoMode;
    unsigned long lastWatering;
    unsigned long lastMeasurement;
    
    // Safety monitoring
    uint8_t pumpCyclesThisHour;
    unsigned long lastHourReset;
    bool sensorError;
    uint8_t sensorErrorCount;
    unsigned long pumpStartTime;
    unsigned long lastPumpCycle;
    bool systemReady;
};

struct SystemConfig {
    bool autoMode;
    int moistureThresholdLow;
    int moistureThresholdHigh;
    uint8_t maxPumpCycles;
    unsigned long pumpTimeout;
    unsigned long pumpCooldown;
};

#endif 