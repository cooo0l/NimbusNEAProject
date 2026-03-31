#pragma once

#include <Arduino.h>
#include "SensorManager.h"
#include "UI.h"

class SDLogger {
public:
    bool begin(uint8_t chipSelectPin);
    bool log(unsigned long timestampMs, const SensorData& data, ExportFormat format);
    bool isAvailable() const;

private:
    bool writeCsv(unsigned long timestampMs, const SensorData& data);
    bool writeJson(unsigned long timestampMs, const SensorData& data);
    bool fileExists(const char* filename) const;

    bool available = false;
};
