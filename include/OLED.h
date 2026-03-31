#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

// Encapsulates low-level display drawing for the OLED.
class OLED {
public:
    OLED(uint8_t dc, uint8_t reset, uint8_t cs);

    bool begin();
    void clear();
    void firstPage();
    bool nextPage();
    void drawText(int16_t x, int16_t y, const char* text);
    void drawValueWithUnit(int16_t x, int16_t y, uint16_t value, const char* unit);
    void drawFloatWithUnit(int16_t x, int16_t y, float value, const char* unit,
                           uint8_t decimals = 1, bool prependDegreeSymbol = false);
    void drawFrame(int16_t x, int16_t y, uint8_t width, uint8_t height);
    void drawBox(int16_t x, int16_t y, uint8_t width, uint8_t height);

    U8G2_SSD1306_128X64_NONAME_1_4W_HW_SPI display;
};
