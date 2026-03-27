#include <Arduino.h>
#include "SensorManager.h"
#include "OLED.h"

namespace {
constexpr uint8_t OLED_DC = 9;
constexpr uint8_t OLED_CS = 10;
constexpr uint8_t OLED_RESET = 8;

SensorManager sensors;
OLED screen(OLED_DC, OLED_RESET, OLED_CS);

unsigned long lastSensorUpdate = 0;
constexpr unsigned long SENSOR_INTERVAL_MS = 1000;
}

void setup() {
    Serial.begin(9600);

    sensors.begin();

    if (!screen.begin()) {
        Serial.println("ERROR: OLED screen failed to start");
        while (true) {
        }
    }

    screen.showLoading();
}

void loop() {
    unsigned long now = millis();

    if (now - lastSensorUpdate >= SENSOR_INTERVAL_MS) {
        lastSensorUpdate = now;
        sensors.update();

        SensorData data = sensors.getData();
        screen.showSensorData(data);

        if (data.dhtValid) {
            Serial.print("Temp: ");
            Serial.print(data.temperature);
            Serial.print(" C  Hum: ");
            Serial.print(data.humidity);
            Serial.println(" %");
        } else {
            Serial.println("DHT22 reading unavailable");
        }

        if (data.ccsValid) {
            Serial.print("CO2: ");
            Serial.print(data.CO2);
            Serial.print(" ppm  TVOC: ");
            Serial.print(data.TVOC);
            Serial.println(" ppb");
        } else {
            Serial.println("CCS811 reading unavailable");
        }
    }
}
