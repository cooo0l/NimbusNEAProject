//#define RE
#ifndef RE
#include <Arduino.h>

// Defining pins
#define CLK 2 // Rotation signal
#define DT 3 // Direction signal
#define SW 4 // Push button

// Stores prev state of CLK
int lastCLK{};

void setup() {
    pinMode(CLK,INPUT_PULLUP);
    pinMode(DT,INPUT_PULLUP);
    pinMode(SW, INPUT_PULLUP);

    Serial.begin(9600);

    lastCLK = digitalRead(CLK); // Captures current state of CLK
}

void loop() {
  int currentCLK = digitalRead(CLK); // Gets the current signal level

  if (currentCLK != lastCLK) { // Checks if there's a change
    if (digitalRead(DT) != currentCLK) { // Checks the direction
      Serial.println("Clockwise");
    } else {
      Serial.println("Anticlockwise");
    }
  }

  lastCLK = currentCLK; // Updates the last state for next loop

  if (digitalRead(SW) == LOW) { // When pressed SW is low
    Serial.println("Button Pressed");
    delay(200); // basic debounce
  }
}

#endif
