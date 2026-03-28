#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Encapsulates low-level display drawing for the OLED.
class OLED {
public:
    OLED(uint8_t dc, uint8_t reset, uint8_t cs);

    bool begin();
    void clear();
    void clearBuffer();
    void present();
    void drawText(int16_t x, int16_t y, const char* text);
    void drawValueWithUnit(int16_t x, int16_t y, uint16_t value, const char* unit);
    void drawFloatWithUnit(int16_t x, int16_t y, float value, const char* unit,
                           uint8_t decimals = 1, bool prependDegreeSymbol = false);

private:
    static const uint8_t SCREEN_WIDTH = 128;
    static const uint8_t SCREEN_HEIGHT = 64;

    Adafruit_SSD1306 display;
};
