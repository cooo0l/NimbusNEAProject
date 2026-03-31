#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_CCS811.h>
#include <DHT.h>

namespace {
constexpr uint8_t DHT_PIN = 4;
constexpr uint8_t DHT_TYPE = DHT22;
constexpr uint8_t I2C_SDA = 21;
constexpr uint8_t I2C_SCL = 22;
constexpr unsigned long READ_INTERVAL_MS = 2000;

Adafruit_CCS811 ccs;
DHT dht(DHT_PIN, DHT_TYPE);
unsigned long lastRead = 0;
bool ccsReady = false;
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("Sensor check starting...");

    Wire.begin(I2C_SDA, I2C_SCL);
    dht.begin();

    ccsReady = ccs.begin();
    if (!ccsReady) {
        Serial.println("CCS811 not found. Check wiring/power.");
    } else {
        Serial.println("CCS811 detected.");
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

        if (ccsReady) {
            ccs.setEnvironmentalData(humidity, temperature);
        }
    }

    if (!ccsReady) {
        Serial.println("CCS811 unavailable.");
    } else if (ccs.available()) {
        if (!ccs.readData()) {
            Serial.print("CCS811 -> CO2: ");
            Serial.print(ccs.geteCO2());
            Serial.print(" ppm, TVOC: ");
            Serial.print(ccs.getTVOC());
            Serial.println(" ppb");
        } else {
            Serial.println("CCS811 read error.");
        }
    } else {
        Serial.println("CCS811 waiting for data...");
    }

    Serial.println();
}
