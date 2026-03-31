#include <Arduino.h>
#include "SensorManager.h"
#include "OLED.h"
#include "RotaryEncoder.h"
#include "UI.h"

namespace {
constexpr uint8_t OLED_DC = 16;
constexpr uint8_t OLED_CS = 5;
constexpr uint8_t OLED_RESET = 17;
constexpr uint8_t ENCODER_CLK = 25;
constexpr uint8_t ENCODER_DT = 26;
constexpr uint8_t ENCODER_SW = 27;
constexpr unsigned long SENSOR_INTERVAL_MS = 1000;

SensorManager sensors;
OLED screen(OLED_DC, OLED_RESET, OLED_CS);
RotaryEncoder encoder(ENCODER_CLK, ENCODER_DT, ENCODER_SW);
UI ui(screen);

unsigned long lastSensorUpdate = 0;

void logSensorData(const SensorData& data) {
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

void setup() {
    Serial.begin(9600);

    sensors.begin();
    encoder.begin();

    if (!screen.begin()) {
        Serial.println("ERROR: OLED screen failed to start");
        while (true) {
        }
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
    }
}
