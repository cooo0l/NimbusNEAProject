#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "SensorManager.h"
#include "UI.h"

class SDLogger {
public:
    bool begin(uint8_t chipSelectPin);
    bool log(unsigned long timestampMs, const SensorData& data, ExportFormat format);
    bool isAvailable() const;
    uint8_t getUsedPercent() const;
    bool retryMount();

private:
    bool writeCsv(unsigned long timestampMs, const SensorData& data);
    bool writeJson(unsigned long timestampMs, const SensorData& data);
    bool fileExists(const char* filename) const;
    void buildSessionFilename(char* buffer, size_t bufferSize, const char* extension,
                              uint16_t index) const;
    bool initializeSessionFiles();
    void updateUsageStats();

    uint8_t chipSelectPin = 0;
    bool available = false;
    bool sessionFilesReady = false;
    uint8_t usedPercent = 0;
    char csvFile[24]{};
    char jsonFile[24]{};
};
