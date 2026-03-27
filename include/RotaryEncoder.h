#pragma once
#include <Arduino.h>
// Defines the setup and input detection
// functions and variables for the compiler
class RotaryEncoder {
public:
    RotaryEncoder(uint8_t CLK, uint8_t DT, uint8_t SW);

    void begin();
    void update();

    int getDelta();
    bool wasPressed();

private:
    uint8_t CLK;
    uint8_t DT;
    uint8_t SW;

    int lastCLK = 0;
    int delta = 0;
    bool pressed = false;
    bool lastSW = HIGH;
    unsigned long lastTurnTime = 0;
    unsigned long lastButtonTime = 0;
};
