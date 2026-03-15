// --- Preprocessor directive to turn on and off compilation of this file

#define OLED
#ifndef OLED

// SPI protocol being used
#include <SPI.h>
// These control communication and drawing to the board respectively
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_DC     9
#define OLED_CS     10
#define OLED_RESET  8

// Instance of OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, OLED_RESET, OLED_CS);

void setup() {
  if (!display.begin(SSD1306_SWITCHCAPVCC)) { // Stop if display fails
    Serial.println("ERROR: OLED screen failed to start"); 
    while (true); 
  }

  display.clearDisplay(); 
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println("Hello World!");
  display.display();
}

void loop() {
}
#endif
 
