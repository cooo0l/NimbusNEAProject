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
constexpr uint8_t MAX_SMOOTHING_SAMPLES = 8;

SensorManager sensors;
OLED screen(OLED_DC, OLED_RESET, OLED_CS);
RotaryEncoder encoder(ENCODER_CLK, ENCODER_DT, ENCODER_SW);
SDLogger sdLogger;
UI ui(screen);

unsigned long lastSensorUpdate = 0;
SensorData recentReadings[MAX_SMOOTHING_SAMPLES]{};
uint8_t nextReadingIndex = 0;
uint8_t recentReadingCount = 0;
// Creates the array of values to be used in the smoothed value
void storeReading(const SensorData& data) {
    recentReadings[nextReadingIndex] = data;
    nextReadingIndex = uint8_t((nextReadingIndex + 1) % MAX_SMOOTHING_SAMPLES);
    if (recentReadingCount < MAX_SMOOTHING_SAMPLES) {
        ++recentReadingCount;
    }
}
// Builds the final value to be displayed
SensorData buildDisplayData(const SensorData& latest, uint8_t sampleCount) {
    // No smoothing
    if (sampleCount <= 1 || recentReadingCount == 0) {
        return latest;
    }
    // If not enough samples for window size, use the available samples
    const uint8_t samplesToUse =
        sampleCount < recentReadingCount ? sampleCount : recentReadingCount;
    SensorData smoothed{};
    float temperatureSum = 0.0f;
    float humiditySum = 0.0f;
    uint32_t co2Sum = 0;
    uint32_t tvocSum = 0;
    uint8_t dhtCount = 0;
    uint8_t ccsCount = 0;
    // Updating totals
    for (uint8_t i = 0; i < samplesToUse; ++i) {
        const uint8_t index =
            uint8_t((nextReadingIndex + MAX_SMOOTHING_SAMPLES - 1 - i) % MAX_SMOOTHING_SAMPLES);
        const SensorData& reading = recentReadings[index];

        if (reading.dhtValid) {
            temperatureSum += reading.temperature;
            humiditySum += reading.humidity;
            ++dhtCount;
        }

        if (reading.ccsValid) {
            co2Sum += reading.CO2;
            tvocSum += reading.TVOC;
            ++ccsCount;
        }
    }
    // Creating the smoothed values
    smoothed.dhtValid = dhtCount > 0;
    if (smoothed.dhtValid) {
        smoothed.temperature = temperatureSum / dhtCount;
        smoothed.humidity = humiditySum / dhtCount;
    }

    smoothed.ccsValid = ccsCount > 0;
    if (smoothed.ccsValid) {
        smoothed.CO2 = uint16_t(co2Sum / ccsCount);
        smoothed.TVOC = uint16_t(tvocSum / ccsCount);
    }

    return smoothed;
}
}
// Setup all components, UI and set pin modes
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
    // Get time and update encoder
    const unsigned long now = millis();
    encoder.update();
    // Move based on encoder status
    ui.moveSelection(encoder.getDelta());

    if (encoder.wasPressed()) {
        ui.select();
    }
    // Update sensor readings
    if (now - lastSensorUpdate >= ui.getReadingIntervalMs()) {
        lastSensorUpdate = now;
        sensors.update();

        const SensorData rawData = sensors.getData();
        storeReading(rawData);
        const SensorData displayData = buildDisplayData(rawData, ui.getSmoothingSampleCount());
        ui.updateSensorData(displayData);

        if (ui.isLoggingEnabled()) {
            digitalWrite(OLED_CS, HIGH);
            const bool writeOk = sdLogger.log(now, rawData, ui.getExportFormat());
            ui.updateStorageStatus(sdLogger.isAvailable(), true, writeOk);
        } else {
            ui.updateStorageStatus(sdLogger.isAvailable(), false, false);
        }
    }
}
