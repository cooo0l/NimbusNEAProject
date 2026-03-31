#include <Arduino.h>
#include "SensorManager.h"
#include "OLED.h"
#include "RotaryEncoder.h"
#include "SDLogger.h"
#include "UI.h"

namespace {
constexpr uint8_t OLED_DC = 16;
constexpr uint8_t OLED_CS = 5;
constexpr uint8_t OLED_RESET = 17;
constexpr uint8_t SD_CS = 13;
constexpr uint8_t ENCODER_CLK = 25;
constexpr uint8_t ENCODER_DT = 26;
constexpr uint8_t ENCODER_SW = 27;
constexpr unsigned long SENSOR_INTERVAL_MS = 1000;

SensorManager sensors;
OLED screen(OLED_DC, OLED_RESET, OLED_CS);
RotaryEncoder encoder(ENCODER_CLK, ENCODER_DT, ENCODER_SW);
SDLogger sdLogger;
UI ui(screen);

unsigned long lastSensorUpdate = 0;
}

void setup() {
    pinMode(OLED_CS, OUTPUT);
    digitalWrite(OLED_CS, HIGH);

    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);

    const bool sdReady = sdLogger.begin(SD_CS);

    if (!screen.begin()) {
        while (true) {
        }
    }

    sensors.begin();
    encoder.begin();

    ui.updateStorageStatus(sdReady, false, false);
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

        if (ui.isLoggingEnabled()) {
            digitalWrite(OLED_CS, HIGH);
            const bool writeOk = sdLogger.log(now, data, ui.getExportFormat());
            ui.updateStorageStatus(sdLogger.isAvailable(), true, writeOk);
        } else {
            ui.updateStorageStatus(sdLogger.isAvailable(), false, false);
        }
    }
}
