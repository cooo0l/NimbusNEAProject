#define MAIN
#ifndef MAIN

#include <Arduino.h>
// Controls the DHT22 sensor
#include <DHT.h>
// Library for CCS811 sensor compatibility
#include <Adafruit_CCS811.h>
// These control communication and drawing to the board respectively
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>


// --- DHT defines
#define DHTPIN 7
#define DHTTYPE DHT22

// --- OLED defines
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_DC     9
#define OLED_CS     10
#define OLED_RESET  8

// ccs is an instance of this class
Adafruit_CCS811 ccs;

// Instance of OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, OLED_RESET, OLED_CS);

// DHT22 sensor object instantiation
DHT DHT_22(DHTPIN,DHTTYPE);



void setup(){
    // Initialise serial communication
    Serial.begin(9600);


    Serial.println("CCS811 Startup...");
    // Error handling message
    if(!ccs.begin()) {
        Serial.println("Sensor failed to start");
        while(1);
    }
    while(!ccs.available());


    Serial.println("DHT22 Startup...");
    DHT_22.begin();

    if (!display.begin(SSD1306_SWITCHCAPVCC)) { // Stop if display fails
        Serial.println("ERROR: OLED screen failed to start"); 
        while (true);
    }
    display.clearDisplay(); 
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    display.println("Loading...");
    display.display();

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
    // Clears the oled for next use
    display.clearDisplay(); 
    // Set cursor to desired postion in pixels (where 0,0 is top left)
    display.setCursor(10, 10);
    display.println("CO2:");
    display.setCursor(40, 10);
    display.print(CCS811_CO2);
    display.println("PPM");
    display.setCursor(10, 24);
    display.println("TVOC:"); 
    display.setCursor(40, 24);
    display.print(CCS811_TVOC);
    display.println("PPB");
    display.setCursor(10, 38);
    display.println("HUM:");
    display.setCursor(40, 38);
    display.print(DHT_22_Humidity);
    display.println("%");
    display.setCursor(10, 52);
    display.println("TEMP:");
    display.setCursor(40, 52);
    display.print(DHT_22_Temperature);
    display.print(char(247));
    display.println("C");
    display.display();

    delay(5000);
}
#endif
