#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_CCS811.h>
#include <DHT.h>

namespace {
constexpr uint8_t DHT_PIN = 32;
constexpr uint8_t DHT_TYPE = DHT22;
constexpr uint8_t I2C_SDA = 21;
constexpr uint8_t I2C_SCL = 22;
constexpr uint8_t CCS811_ADDR = 0x5A;
constexpr uint8_t CCS811_ERROR_ID_REG = 0xE0;
constexpr unsigned long READ_INTERVAL_MS = 5000;
constexpr unsigned long CCS811_STARTUP_DELAY_MS = 5000;

Adafruit_CCS811 ccs;
DHT dht(DHT_PIN, DHT_TYPE);
unsigned long lastRead = 0;
bool ccsReady = false;

uint8_t readCcs811Register(uint8_t reg) {
    Wire.beginTransmission(CCS811_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
        return 0xFF;
    }

    if (Wire.requestFrom(static_cast<int>(CCS811_ADDR), 1) != 1) {
        return 0xFF;
    }

    return Wire.read();
}

void printCcs811ErrorFlags(uint8_t errorId) {
    if (errorId == 0xFF) {
        Serial.println("Could not read CCS811 ERROR_ID register.");
        return;
    }

    Serial.print("CCS811 ERROR_ID: 0x");
    if (errorId < 0x10) {
        Serial.print('0');
    }
    Serial.println(errorId, HEX);

    if (errorId == 0) {
        Serial.println("Status ERROR bit is set, but ERROR_ID is 0.");
    }
    if (errorId & 0x01) {
        Serial.println(" - WRITE_REG_INVALID");
    }
    if (errorId & 0x02) {
        Serial.println(" - READ_REG_INVALID");
    }
    if (errorId & 0x04) {
        Serial.println(" - MEASMODE_INVALID");
    }
    if (errorId & 0x08) {
        Serial.println(" - MAX_RESISTANCE");
    }
    if (errorId & 0x10) {
        Serial.println(" - HEATER_FAULT");
    }
    if (errorId & 0x20) {
        Serial.println(" - HEATER_SUPPLY");
    }
}
}

void setup() {
    Serial.begin(9600);
    delay(1000);

    Serial.println();
    Serial.println("Sensor check starting...");

    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(100000);
    dht.begin();

    ccsReady = ccs.begin();
    if (!ccsReady) {
        Serial.println("CCS811 not found. Check wiring/power.");
    } else {
        Serial.println("CCS811 detected.");
        ccs.setDriveMode(CCS811_DRIVE_MODE_1SEC);
        Serial.print("Waiting ");
        Serial.print(CCS811_STARTUP_DELAY_MS);
        Serial.println(" ms for first CCS811 sample...");
        delay(CCS811_STARTUP_DELAY_MS);
    }
}

void loop() {
    const unsigned long now = millis();
    if (now - lastRead < READ_INTERVAL_MS) {
        return;
    }
    lastRead = now;

    const float temperature = dht.readTemperature();
    const float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("DHT22 read failed.");
    } else {
        Serial.print("DHT22 -> Temp: ");
        Serial.print(temperature, 1);
        Serial.print(" C, Humidity: ");
        Serial.print(humidity, 1);
        Serial.println(" %");
/*
        if (ccsReady) {
            ccs.setEnvironmentalData(humidity, temperature);
        }*/
    }

    if (!ccsReady) {
        Serial.println("CCS811 unavailable.");
    } else if (ccs.available()) {
        const uint8_t ccsError = ccs.readData();
        if (!ccsError) {
            Serial.print("CCS811 -> CO2: ");
            Serial.print(ccs.geteCO2());
            Serial.print(" ppm, TVOC: ");
            Serial.print(ccs.getTVOC());
            Serial.println(" ppb");
        } else {
            Serial.print("CCS811 read error. ERROR_ID: 0x");
            Serial.println(ccsError, HEX);
        }
    } else {
        Serial.print("CCS811 waiting for data");
        if (ccs.checkError()) {
            Serial.println(" and status reports an internal sensor error.");
            printCcs811ErrorFlags(readCcs811Register(CCS811_ERROR_ID_REG));
        } else {
            Serial.println(" (sensor detected, but DATA_READY is still low).");
        }
    }

    Serial.println();
}
/*
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
unsigned long lastInteractionTime = 0;
SensorData recentReadings[MAX_SMOOTHING_SAMPLES]{};
uint8_t nextReadingIndex = 0;
uint8_t recentReadingCount = 0;
bool displaySleeping = false;
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
    lastInteractionTime = millis();
}

void loop() {
    // Get time and update encoder
    const unsigned long now = millis();
    encoder.update();
    const int encoderDelta = encoder.getDelta();
    const bool encoderPressed = encoder.wasPressed();

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
    }

    const unsigned long autoSleepTimeout = ui.getAutoSleepTimeoutMs();
    if (!displaySleeping && autoSleepTimeout > 0 && now - lastInteractionTime >= autoSleepTimeout) {
        screen.setSleep(true);
        displaySleeping = true;
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
*/
