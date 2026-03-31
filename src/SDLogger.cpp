#include "SDLogger.h"
#include <SD.h>

namespace {
const char CSV_FILE[] = "readings.csv";
const char JSON_FILE[] = "readings.txt";
}

bool SDLogger::begin(uint8_t chipSelectPin) {
    pinMode(chipSelectPin, OUTPUT);
    digitalWrite(chipSelectPin, HIGH);

    available = SD.begin(chipSelectPin);
    return available;
}

bool SDLogger::log(unsigned long timestampMs, const SensorData& data, ExportFormat format) {
    if (!available) {
        return false;
    }

    if (format == ExportFormat::Json) {
        return writeJson(timestampMs, data);
    }

    return writeCsv(timestampMs, data);
}

bool SDLogger::isAvailable() const {
    return available;
}

bool SDLogger::writeCsv(unsigned long timestampMs, const SensorData& data) {
    const bool needsHeader = !fileExists(CSV_FILE);
    File file = SD.open(CSV_FILE, FILE_WRITE);
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
    file.close();
    return true;
}

bool SDLogger::writeJson(unsigned long timestampMs, const SensorData& data) {
    File file = SD.open(JSON_FILE, FILE_WRITE);
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
    file.close();
    return true;
}

bool SDLogger::fileExists(const char* filename) const {
    return SD.exists(filename);
}
