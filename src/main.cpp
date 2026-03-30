#include <Arduino.h>
#include "SensorManager.h"
#include "OLED.h"
#include "RotaryEncoder.h"
#include "SDLogger.h"
#include "UI.h"

namespace {
constexpr uint8_t OLED_DC = 9;
constexpr uint8_t OLED_CS = 10;
constexpr uint8_t OLED_RESET = 8;
constexpr uint8_t SD_CS = 5;
constexpr uint8_t ENCODER_CLK = 2;
constexpr uint8_t ENCODER_DT = 3;
constexpr uint8_t ENCODER_SW = 4;
constexpr unsigned long SENSOR_INTERVAL_MS = 1000;

SensorManager sensors;
OLED screen(OLED_DC, OLED_RESET, OLED_CS);
RotaryEncoder encoder(ENCODER_CLK, ENCODER_DT, ENCODER_SW);
SDLogger sdLogger;
UI ui(screen);

unsigned long lastSensorUpdate = 0;

void logSensorData(const SensorData& data) {
    if (data.dhtValid) {
        Serial.print(F("Temp: "));
        Serial.print(data.temperature);
        Serial.print(F(" C  Hum: "));
        Serial.print(data.humidity);
        Serial.println(F(" %"));
    } else {
        Serial.println(F("DHT22 reading unavailable"));
    }

    if (data.ccsValid) {
        Serial.print(F("CO2: "));
        Serial.print(data.CO2);
        Serial.print(F(" ppm  TVOC: "));
        Serial.print(data.TVOC);
        Serial.println(F(" ppb"));
    } else {
        Serial.println(F("CCS811 reading unavailable"));
    }
}
}

void setup() {
    Serial.begin(9600);

    if (!screen.begin()) {
        Serial.println(F("ERROR: OLED screen failed to start"));
        while (true) {
        }
    }

    pinMode(OLED_CS, OUTPUT);
    digitalWrite(OLED_CS, HIGH);

    encoder.begin();
    sensors.begin();

    if (!sdLogger.begin(SD_CS)) {
        Serial.println(F("WARNING: SD card init failed"));
    }

    ui.begin();
}

void loop() {
    const unsigned long now = millis();
    encoder.update();

    ui.moveSelection(encoder.getDelta());

    if (encoder.wasPressed()) {
        ui.select();
    }

    if (now - lastSensorUpdate >= SENSOR_INTERVAL_MS) {
        lastSensorUpdate = now;
        sensors.update();

        const SensorData data = sensors.getData();
        ui.updateSensorData(data);
        logSensorData(data);

        if (ui.isLoggingEnabled()) {
            if (!sdLogger.log(now, data, ui.getExportFormat())) {
                Serial.println(F("WARNING: SD write failed"));
            }
        }
    }
}
