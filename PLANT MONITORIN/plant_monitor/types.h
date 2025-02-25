#ifndef TYPES_H
#define TYPES_H

struct SystemState {
    float soilMoisture;
    float temperature;
    float humidity;
    bool pumpActive;
    bool autoMode;
    unsigned long lastWatering;
    unsigned long lastMeasurement;
};

struct SystemConfig {
    bool autoMode;
    int moistureThresholdLow;
    int moistureThresholdHigh;
};

#endif 