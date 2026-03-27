#include <Arduino.h>
#include "SensorManager.h"

SensorManager sensors;

void setup() {
    Serial.begin(9600);
    sensors.begin();
}

void loop() {
    sensors.update();

    SensorData data = sensors.getData();

    Serial.println("---- Sensor Readings ----");

    if (data.dhtValid) {
        Serial.print("Temperature: ");
        Serial.println(data.temperature);

        Serial.print("Humidity: ");
        Serial.println(data.humidity);
    } else {
        Serial.println("DHT read failed");
    }

    if (data.ccsValid) {
        Serial.print("CO2: ");
        Serial.println(data.CO2);

        Serial.print("TVOC: ");
        Serial.println(data.TVOC);
    } else {
        Serial.println("CCS811 read failed or no new data");
    }

    delay(2000);
}
