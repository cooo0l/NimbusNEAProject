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
