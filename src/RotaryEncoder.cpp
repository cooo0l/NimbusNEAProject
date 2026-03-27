#include "RotaryEncoder.h"

// Constructor for class RotaryEncoder, and initialise
// class members with the constructor argument
RotaryEncoder::RotaryEncoder(uint8_t clk, uint8_t dt, uint8_t sw)
    : CLK(clk), DT(dt), SW(sw) {}

// Startup for the rotary encoder
void RotaryEncoder::begin() {
    pinMode(CLK, INPUT_PULLUP);
    pinMode(DT, INPUT_PULLUP);
    pinMode(SW, INPUT_PULLUP);

    // Stores previous state of CLK
    // and SW now
    lastCLK = digitalRead(CLK);
    lastSW = digitalRead(SW);
}

// Detecting input
void RotaryEncoder::update() {
    int currentCLK = digitalRead(CLK);

    // Detect a change on CLK
    if (currentCLK != lastCLK) {
        // Determine direction by comparing DT with CLK
        if (digitalRead(DT) != currentCLK) {
            delta++;
        } else {
            delta--;
        }
    }

    lastCLK = currentCLK;

    // Button press detection
    bool currentButtonState = digitalRead(SW);
    if (lastSW == HIGH && currentButtonState == LOW) {
        pressed = true;
    }
    lastSW = currentButtonState;
}
// Returns the rotation amount
int RotaryEncoder::getDelta() {
    int result = delta;
    delta = 0;
    return result;
}
// Returns if button was pressed
bool RotaryEncoder::wasPressed() {
    bool result = pressed;
    pressed = false;
    return result;
}
