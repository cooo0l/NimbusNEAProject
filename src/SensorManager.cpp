#include "SensorManager.h"
#include <Wire.h>
#include <Adafruit_CCS811.h>
#include <DHT.h>

namespace {
constexpr uint8_t DHT_PIN = 32;
constexpr uint8_t DHT_TYPE = DHT22;
constexpr uint8_t I2C_SDA = 21;
constexpr uint8_t I2C_SCL = 22;
constexpr uint8_t CCS811_ADDR_PRIMARY = 0x5A;
constexpr uint8_t CCS811_ADDR_SECONDARY = 0x5B;
constexpr unsigned long CCS811_STARTUP_DELAY_MS = 5000;

Adafruit_CCS811 ccs;
DHT dht(DHT_PIN, DHT_TYPE);
}

void SensorManager::begin() {
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(100000);
    dht.begin();
    currentData.ccsValid = false;
    currentData.dhtValid = false;

    ccsDetected = ccs.begin(CCS811_ADDR_PRIMARY, &Wire);
    if (ccsDetected) {
        ccsAddress = CCS811_ADDR_PRIMARY;
    } else {
        ccsDetected = ccs.begin(CCS811_ADDR_SECONDARY, &Wire);
        if (ccsDetected) {
            ccsAddress = CCS811_ADDR_SECONDARY;
        }
    }

    if (!ccsDetected) {
        return;
    }

    ccs.setDriveMode(CCS811_DRIVE_MODE_1SEC);
    ccsStartupTimeMs = millis();
}

void SensorManager::update() {
    const float temp = dht.readTemperature();
    const float hum = dht.readHumidity();

    if (!isnan(temp) && !isnan(hum)) {
        currentData.temperature = temp;
        currentData.humidity = hum;
        currentData.dhtValid = true;

        if (ccsDetected) {
            ccs.setEnvironmentalData(hum, temp);
        }
    } else {
        currentData.dhtValid = false;
    }

    if (!ccsDetected) {
        currentData.ccsValid = false;
        return;
    }

    if (millis() - ccsStartupTimeMs < CCS811_STARTUP_DELAY_MS) {
        currentData.ccsValid = false;
        return;
    }

    if (ccs.available()) {
        if (ccs.readData() == 0) {
            currentData.CO2 = ccs.geteCO2();
            currentData.TVOC = ccs.getTVOC();
            currentData.ccsValid = true;
        } else {
            currentData.ccsValid = false;
        }
    } else {
        currentData.ccsValid = false;
    }
}

SensorData SensorManager::getData() const {
    return currentData;
}
