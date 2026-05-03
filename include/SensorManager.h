// Prevents file from being included multiple times
// More robust than traditional header guard
#pragma once
#include <Arduino.h>

// All readings are stored in one object
struct SensorData {
    // Initialise variables to store readings
    uint16_t CO2{};
    uint16_t TVOC{};
    float temperature{};
    float humidity{};

    bool ccsValid = false;
    bool dhtValid = false;
};

class SensorManager {
public:
    void begin();
    void update();
    SensorData getData() const;

private:
    SensorData currentData;
    bool ccsDetected = false;
    unsigned long ccsStartupTimeMs = 0;
    uint8_t ccsAddress = 0;
};
