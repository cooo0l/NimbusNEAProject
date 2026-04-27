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

    // Detect a single press and then ignore until the button is released.
    bool currentButtonState = digitalRead(SW);
    unsigned long now = millis();

    if (buttonArmed && lastSW == HIGH && currentButtonState == LOW) {
        if (now - lastButtonTime > 50) {
            pressed = true;
            buttonArmed = false;
            lastButtonTime = now;
        }
    }

    // Re-arm after a stable release.
    if (!buttonArmed && lastSW == LOW && currentButtonState == HIGH) {
        if (now - lastButtonTime > 50) {
            buttonArmed = true;
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
