#include "SensorManager.h"
#include <Wire.h>
#include <Adafruit_CCS811.h>
#include <DHT.h>

// DHT defines
#define DHTPIN 7
#define DHTTYPE DHT22

// Instantiate library instances
static Adafruit_CCS811 ccs;
static DHT dht(DHTPIN, DHTTYPE);

// Sensor startup
void SensorManager::begin() {
    Wire.begin();
    dht.begin();

    if (!ccs.begin()) {
        Serial.println("CCS811 init failed"); // Error handling message
    }
}
// Updates currentData using sensors
void SensorManager::update() {
    // DHT22
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    // If both are valid, currentData is updated
    if (!isnan(temp) && !isnan(hum)) { 
        currentData.temperature = temp;
        currentData.humidity = hum;
        currentData.dhtValid = true;

        // Improves readings for CCS811
        // by giving it the temp and humidity
        ccs.setEnvironmentalData(hum, temp);
    } else {
        currentData.dhtValid = false;
    }

    // CCS811
    if (ccs.available()) {
        if (!ccs.readData()) {
            currentData.CO2 = ccs.geteCO2();
            currentData.TVOC = ccs.getTVOC();
            currentData.ccsValid = true;
        } else {
            currentData.ccsValid = false;
        }
    }
}

SensorData SensorManager::getData() const {
    return currentData;
}
