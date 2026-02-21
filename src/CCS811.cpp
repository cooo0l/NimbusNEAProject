/*

#include <Arduino.h>
// Library for CCS811 sensor compatibility
#include <Adafruit_CCS811.h>


// ccs is an instance of this class

Adafruit_CCS811 ccs;

void setup() {

    // Intitialises serial communication aaaa
    // between the Arduino and computer
    Serial.begin(9600);
    Serial.println("CCS811");

    // Error handling message, usually an caused by an issue with wiring
    if(!ccs.begin()) {
        Serial.println("Sensor failed to start");
        while(1);
    }

    Serial.println("Sensor started successfully");

    // This lines makes execution idle until the sensor
    // is ready to take readings
    while(!ccs.available());
}

void loop() {
    // Continuosly print out both TVOC and CO2 readings
    if(!ccs.readData()){
        Serial.print("CO2: ");
        Serial.print(ccs.geteCO2());
        Serial.print("ppm, TVOC: ");
        Serial.println(ccs.getTVOC());
    }
    else{
        Serial.println("Error no readings produced");
        while(1);
    }


    // This is the time interval between readings
    delay(5000);
}

*/
