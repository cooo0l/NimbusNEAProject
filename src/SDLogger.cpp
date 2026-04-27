#include "SDLogger.h"
#include <SD.h>

namespace {
const char CSV_FILE[] = "/readings.csv";
const char JSON_FILE[] = "/readings.txt";
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
    return available;
}
// Takes in data, timestamp and file format, and calls
// the appropriate logging function
bool SDLogger::log(unsigned long timestampMs, const SensorData& data, ExportFormat format) {
    if (!available) {
        return false;
    }

    if (format == ExportFormat::Json) {
        return writeJson(timestampMs, data);
    }

    return writeCsv(timestampMs, data);
}
// Returns if sd logging is available
bool SDLogger::isAvailable() const {
    return available;
}
// Writes a CSV record to the SD card
bool SDLogger::writeCsv(unsigned long timestampMs, const SensorData& data) {
    const bool needsHeader = !fileExists(CSV_FILE); 
    File file = SD.open(CSV_FILE, FILE_APPEND); 
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
    File file = SD.open(JSON_FILE, FILE_APPEND);
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
