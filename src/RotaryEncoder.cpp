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

    // Only count the falling edge to reduce duplicate steps
    // Only check 5ms after previous reading to reduce noise
    if (lastCLK == HIGH && currentCLK == LOW) {
        unsigned long now = millis();
        if (now - lastTurnTime > 5) {
            if (digitalRead(DT) == HIGH) {
                delta++;
            } else {
                delta--;
            }
            lastTurnTime = now;
        }
    }

    lastCLK = currentCLK;

    // Detect a button press once and ignore switch bounce.
    bool currentButtonState = digitalRead(SW);
    if (lastSW == HIGH && currentButtonState == LOW) {
        unsigned long now = millis();
        if (now - lastButtonTime > 50) {
            pressed = true;
            lastButtonTime = now;
        }
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
