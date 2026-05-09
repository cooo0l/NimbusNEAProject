#include "SDLogger.h"
#include <SD.h>
#include <stdio.h>

namespace {
constexpr uint8_t SD_SCK = 18;
constexpr uint8_t SD_MISO = 19;
constexpr uint8_t SD_MOSI = 23;
constexpr uint32_t SD_SPI_HZ = 1000000;
}

bool SDLogger::begin(uint8_t chipSelectPin) {
    this->chipSelectPin = chipSelectPin;
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, chipSelectPin);
    pinMode(chipSelectPin, OUTPUT);
    digitalWrite(chipSelectPin, HIGH);

    available = SD.begin(chipSelectPin, SPI, SD_SPI_HZ);
    sessionFilesReady = false;
    if (available) {
        // Each boot session gets its own log file so runs are kept separate
        sessionFilesReady = initializeSessionFiles();
        if (!sessionFilesReady) {
            available = false;
            SD.end();
        } else {
            updateUsageStats();
        }
    }
    return available;
}
// Takes in data, timestamp and file format, and calls
// the appropriate logging function
bool SDLogger::log(unsigned long timestampMs, const SensorData& data, ExportFormat format) {
    if (!available || !sessionFilesReady) {
        return false;
    }

    const bool writeOk = format == ExportFormat::Json ? writeJson(timestampMs, data)
                                                      : writeCsv(timestampMs, data);
    if (!writeOk) {
        available = false;
        sessionFilesReady = false;
        usedPercent = 0;
        SD.end();
    } else {
        updateUsageStats();
    }
    return writeOk;
}
// Returns if sd logging is available
bool SDLogger::isAvailable() const {
    return available;
}

bool SDLogger::retryMount() {
    if (available) {
        return true;
    }
    if (chipSelectPin == 0) {
        return false;
    }
    // Remount attempts are only triggered manually from the storage screen to
    // avoid periodic SD polling freezing the encoder/UI loop
    return begin(chipSelectPin);
}
// Returns the percentage of storage used
uint8_t SDLogger::getUsedPercent() const {
    return usedPercent;
}
// Writes a CSV record to the SD card
bool SDLogger::writeCsv(unsigned long timestampMs, const SensorData& data) {
    const bool needsHeader = !fileExists(csvFile); 
    File file = SD.open(csvFile, FILE_APPEND); 
    if (!file) { 
        return false;
    }

    if (needsHeader) {
        file.println(F("time_ms,temp_c,humidity_pct,co2_ppm,tvoc_ppb,dht_valid,ccs_valid"));
    }

    file.print(timestampMs); 
    file.print(','); 
    if (data.dhtValid) { 
        file.print(data.temperature, 1);
    }
    file.print(','); 
    if (data.dhtValid) { 
        file.print(data.humidity, 1);
    } 
    file.print(','); 
    if (data.ccsValid) { 
        file.print(data.CO2);
    } 
    file.print(','); 
    if (data.ccsValid) { 
        file.print(data.TVOC);
    } 
    file.print(','); 
    file.print(data.dhtValid ? 1 : 0); 
    file.print(','); 
    file.println(data.ccsValid ? 1 : 0); 
    file.flush();
    file.close();
    return true;
}
// Writes a JSON record to the SD card
bool SDLogger::writeJson(unsigned long timestampMs, const SensorData& data) {
    File file = SD.open(jsonFile, FILE_APPEND);
    if (!file) {
        return false;
    }

    file.print(F("{\"time_ms\":"));
    file.print(timestampMs);
    file.print(F(",\"temperature\":"));
    if (data.dhtValid) {
        file.print(data.temperature, 1);
    } else {
        file.print(F("null"));
    }
    file.print(F(",\"humidity\":"));
    if (data.dhtValid) {
        file.print(data.humidity, 1);
    } else {
        file.print(F("null"));
    }
    file.print(F(",\"co2\":"));
    if (data.ccsValid) {
        file.print(data.CO2);
    } else {
        file.print(F("null"));
    }
    file.print(F(",\"tvoc\":"));
    if (data.ccsValid) {
        file.print(data.TVOC);
    } else {
        file.print(F("null"));
    }
    file.print(F(",\"dhtValid\":"));
    file.print(data.dhtValid ? F("true") : F("false"));
    file.print(F(",\"ccsValid\":"));
    file.print(data.ccsValid ? F("true") : F("false"));
    file.println(F("}"));
    file.flush();
    file.close();
    return true;
}

bool SDLogger::fileExists(const char* filename) const {
    if (!available) {
        return false;
    }

    return SD.exists(filename);
}

void SDLogger::buildSessionFilename(char* buffer, size_t bufferSize, const char* extension,
                                    uint16_t index) const {
    snprintf(buffer, bufferSize, "/readings%u.%s", index, extension);
}

bool SDLogger::initializeSessionFiles() {
    char candidateCsv[24];
    char candidateJson[24];

    for (uint16_t index = 0; index < 1000; ++index) {
        buildSessionFilename(candidateCsv, sizeof(candidateCsv), "csv", index);
        buildSessionFilename(candidateJson, sizeof(candidateJson), "txt", index);

        // Reserve the next unused pair so one boot cannot overwrite an older session.
        if (!SD.exists(candidateCsv) && !SD.exists(candidateJson)) {
            snprintf(csvFile, sizeof(csvFile), "%s", candidateCsv);
            snprintf(jsonFile, sizeof(jsonFile), "%s", candidateJson);
            return true;
        }
    }

    return false;
}

void SDLogger::updateUsageStats() {
    if (!available) {
        usedPercent = 0;
        return;
    }

    // Cache the percentage so the UI can read it without repeatedly touching the card
    const uint64_t total = SD.totalBytes();
    if (total == 0) {
        usedPercent = 0;
        return;
    }

    const uint64_t used = SD.usedBytes();
    const uint64_t percent = (used * 100ULL) / total;
    usedPercent = percent > 100ULL ? 100U : static_cast<uint8_t>(percent);
}
