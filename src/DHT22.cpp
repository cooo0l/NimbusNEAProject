#include <Arduino.h>
// Controls the DHT22 sensor
#include <DHT.h>

// Preprocessor directives to easily change which pin and 
// module type we're using
#define DHTPIN 7
#define DHTTYPE DHT22

// DHT22 sensor object instantiation
DHT DHT_22(DHTPIN,DHTTYPE);

void setup(){
    // Similar to before, initialise serial communication
    Serial.begin(9600);

    // Because this function doesn't return a boolean
    // we can't use it to error check, simply initialises
    // the sensor
    DHT_22.begin();
}

void loop(){
    // Extracts data from the sensors
    float DHT_22_Humidity=DHT_22.readHumidity();
    float DHT_22_Temperature=DHT_22.readTemperature();  // add true argument 
                                                        // to change to degrees farenheit

    // Format data and print to serial monitor
    Serial.print("Humidity: ");
    Serial.print(DHT_22_Humidity);
    Serial.println("%");
    Serial.print("Temperature: ");
    Serial.print(DHT_22_Temperature);
    Serial.println("°C");

    delay(5000);
}
