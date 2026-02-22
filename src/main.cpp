#include <Arduino.h>
// Controls the DHT22 sensor
#include <DHT.h>
// Library for CCS811 sensor compatibility
#include <Adafruit_CCS811.h>

// Preprocessor directives to easily change which pin and 
// module type we're using
#define DHTPIN 7
#define DHTTYPE DHT22

// ccs is an instance of this class
Adafruit_CCS811 ccs;

// DHT22 sensor object instantiation
DHT DHT_22(DHTPIN,DHTTYPE);



void setup(){
    // Initialise serial communication
    Serial.begin(9600);


    Serial.println("CCS811 Startup...");
    // Error handling message, usually an caused by an issue with wiring
    if(!ccs.begin()) {
        Serial.println("Sensor failed to start");
        while(1);
    }
    while(!ccs.available());


    Serial.println("DHT22 Startup...");
    // Because this function doesn't return a boolean
    // we can't use it to error check, simply initialises
    // the sensor
    DHT_22.begin();


}

void loop(){
    //  ----- Read DHT22 -----
    float DHT_22_Humidity=DHT_22.readHumidity();
    float DHT_22_Temperature=DHT_22.readTemperature();  // add true argument 
                                                        // to change to degrees farenheit

    //  ----- Read CCS811 -----

    float CCS811_CO2 {};
    float CCS811_TVOC {};
          
    if(!ccs.readData()){                               
        CCS811_CO2=ccs.geteCO2();
        CCS811_TVOC=ccs.getTVOC();
    }
    else{
        Serial.println("Error no readings produced");
        while(1);
    }                                                    

    // Format data and print to serial monitor
    Serial.print("Humidity: ");
    Serial.print(DHT_22_Humidity);
    Serial.println("%");
    Serial.print("Temperature: ");
    Serial.print(DHT_22_Temperature);
    Serial.println("°C");

    // Continuosly print out both TVOC and CO2 readings
    Serial.print("CO2: ");
    Serial.print(CCS811_CO2);
    Serial.print("ppm, TVOC: ");
    Serial.println(CCS811_TVOC);
    

    delay(5000);
}
