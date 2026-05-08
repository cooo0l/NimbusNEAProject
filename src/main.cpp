#include <Arduino.h>
#include "SensorManager.h"
#include "OLED.h"
#include "RotaryEncoder.h"
#include "SDLogger.h"
#include "SettingsStorage.h"
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
constexpr bool ENABLE_OLED = true;
constexpr bool ENABLE_ENCODER = true;
constexpr bool ENABLE_SETTINGS_STORAGE = true;
constexpr bool ENABLE_SD = true;
constexpr bool ENABLE_SD_LOGGING = true;
constexpr unsigned long SENSOR_ONLY_INTERVAL_MS = 5000;
constexpr unsigned long BACKGROUND_IDLE_MS = 250;

SensorManager sensors;
OLED screen(OLED_DC, OLED_RESET, OLED_CS);
RotaryEncoder encoder(ENCODER_CLK, ENCODER_DT, ENCODER_SW);
SDLogger sdLogger;
SettingsStorage settingsStorage;
UI ui(screen);

unsigned long lastSensorUpdate = 0;
unsigned long lastInteractionTime = 0;
SensorData recentReadings[MAX_SMOOTHING_SAMPLES]{};
SensorData pendingDisplayData{};
SensorData pendingLogData{};
uint8_t nextReadingIndex = 0;
uint8_t recentReadingCount = 0;
bool displaySleeping = false;
bool settingsStorageReady = false;
bool sensorSamplePending = false;
bool uiUpdatePending = false;
bool sdLogPending = false;
unsigned long pendingLogTimestamp = 0;
Screen lastScreen = Screen::Loading;

bool getSdAvailable() {
    if (!ENABLE_SD) {
        return false;
    }
    return sdLogger.isAvailable();
}
// Creates the array of values to be used in the smoothed value
void storeReading(const SensorData& data) {
    recentReadings[nextReadingIndex] = data;
    nextReadingIndex = uint8_t((nextReadingIndex + 1) % MAX_SMOOTHING_SAMPLES);
    if (recentReadingCount < MAX_SMOOTHING_SAMPLES) {
        ++recentReadingCount;
    }
}
// Builds the final value to be displayed using the latest sensor data and
// how many samples to use for moving average
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
    Serial.begin(9600);
    delay(200);

    sensors.begin();
    Serial.print("Feature flags -> OLED=");
    Serial.print(ENABLE_OLED ? "on" : "off");
    Serial.print(", Encoder=");
    Serial.print(ENABLE_ENCODER ? "on" : "off");
    Serial.print(", Settings=");
    Serial.print(ENABLE_SETTINGS_STORAGE ? "on" : "off");
    Serial.print(", SD=");
    Serial.print(ENABLE_SD ? "on" : "off");
    Serial.print(", SD logging=");
    Serial.println(ENABLE_SD_LOGGING ? "on" : "off");

    bool sdReady = false;

    if (ENABLE_OLED) {
        pinMode(OLED_CS, OUTPUT);
        digitalWrite(OLED_CS, HIGH);

        if (!screen.begin()) {
            while (true) {
            }
        }
    }

    if (ENABLE_SD) {
        pinMode(SD_CS, OUTPUT);
        digitalWrite(SD_CS, HIGH);
        sdReady = sdLogger.begin(SD_CS);
    }

    if (ENABLE_SETTINGS_STORAGE) {
        settingsStorageReady = settingsStorage.begin();
        UISettings loadedSettings;
        if (settingsStorageReady && settingsStorage.load(loadedSettings)) {
            ui.applySettings(loadedSettings);
        }
    }

    if (ENABLE_ENCODER) {
        encoder.begin();
    }

    if (ENABLE_OLED) {
        const bool sdAvailable = ENABLE_SD ? sdReady : false;
        const uint8_t storagePercent = sdAvailable ? sdLogger.getUsedPercent() : 0;
        ui.updateStorageStatus(sdAvailable, false, false, storagePercent);
        ui.begin();
        lastScreen = ui.getCurrentScreen();
        lastInteractionTime = millis();
    } else {
        Serial.println("Minimal mode enabled.");
    }
}

void loop() {
    if (!ENABLE_OLED) {
        const unsigned long now = millis();

        if (now - lastSensorUpdate >= SENSOR_ONLY_INTERVAL_MS) {
            lastSensorUpdate = now;
            sensors.update();

            const SensorData data = sensors.getData();

            if (data.dhtValid) {
                Serial.print("DHT22 -> Temp: ");
                Serial.print(data.temperature, 1);
                Serial.print(" C, Humidity: ");
                Serial.print(data.humidity, 1);
                Serial.println(" %");
            } else {
                Serial.println("DHT22 read failed.");
            }

            if (data.ccsValid) {
                Serial.print("CCS811 -> CO2: ");
                Serial.print(data.CO2);
                Serial.print(" ppm, TVOC: ");
                Serial.print(data.TVOC);
                Serial.println(" ppb");
            } else {
                Serial.println("CCS811 data not valid yet.");
            }

            Serial.println();
        }

        return;
    }

    // Get time and update encoder
    const unsigned long now = millis();
    int encoderDelta = 0;
    bool encoderPressed = false;

    if (ENABLE_ENCODER) {
        encoder.update();
        encoderDelta = encoder.getDelta();
        encoderPressed = encoder.wasPressed();
    }

    if (encoderDelta != 0 || encoderPressed) {
        lastInteractionTime = now;
        if (displaySleeping) {
            screen.setSleep(false);
            displaySleeping = false;
            ui.refresh();
        }
    }

    // Move based on encoder status
    ui.moveSelection(encoderDelta);

    if (encoderPressed) {
        ui.select();
        if (ENABLE_SETTINGS_STORAGE && settingsStorageReady && ui.consumeSettingsChanged()) {
            settingsStorage.save(ui.getSettings());
        }
    }

    const Screen currentScreen = ui.getCurrentScreen();
    if (currentScreen != lastScreen) {
        if (ENABLE_SD && currentScreen == Screen::DataAndStorage) {
            const bool sdAvailable = sdLogger.retryMount();
            const uint8_t storagePercent = sdAvailable ? sdLogger.getUsedPercent() : 0;
            ui.updateStorageStatus(sdAvailable, false, false, storagePercent);
        }
        lastScreen = currentScreen;
    }

    const unsigned long autoSleepTimeout = ui.getAutoSleepTimeoutMs();
    if (!displaySleeping && autoSleepTimeout > 0 && now - lastInteractionTime >= autoSleepTimeout) {
        screen.setSleep(true);
        displaySleeping = true;
    }

    const bool userActive = encoderDelta != 0 || encoderPressed;
    const bool backgroundWindowOpen =
        !userActive && (now - lastInteractionTime >= BACKGROUND_IDLE_MS);
    bool ranSensorSample = false;
    bool ranUiUpdate = false;

    if (!sensorSamplePending && now - lastSensorUpdate >= ui.getReadingIntervalMs()) {
        sensorSamplePending = true;
    }

    if (sensorSamplePending && backgroundWindowOpen) {
        lastSensorUpdate = now;
        sensorSamplePending = false;
        ranSensorSample = true;
        sensors.update();

        const SensorData rawData = sensors.getData();
        storeReading(rawData);
        pendingDisplayData = buildDisplayData(rawData, ui.getSmoothingSampleCount());
        uiUpdatePending = true;

        if (ENABLE_SD && ENABLE_SD_LOGGING && ui.isLoggingEnabled()) {
            pendingLogData = rawData;
            pendingLogTimestamp = now;
            sdLogPending = true;
        }
    }

    if (uiUpdatePending && backgroundWindowOpen && !ranSensorSample) {
        ui.updateSensorData(pendingDisplayData);
        if (!(ENABLE_SD && ENABLE_SD_LOGGING && ui.isLoggingEnabled())) {
            const bool sdAvailable = getSdAvailable();
            const uint8_t storagePercent = sdAvailable ? sdLogger.getUsedPercent() : 0;
            ui.updateStorageStatus(sdAvailable, false, false, storagePercent);
        }
        uiUpdatePending = false;
        ranUiUpdate = true;
    }

    if (sdLogPending && backgroundWindowOpen && !ranSensorSample && !ranUiUpdate) {
        digitalWrite(OLED_CS, HIGH);
        const bool writeOk = sdLogger.log(pendingLogTimestamp, pendingLogData, ui.getExportFormat());
        const bool sdAvailable = getSdAvailable();
        const uint8_t storagePercent = sdAvailable ? sdLogger.getUsedPercent() : 0;
        ui.updateStorageStatus(sdAvailable, true, writeOk, storagePercent);
        sdLogPending = false;
    }
}
