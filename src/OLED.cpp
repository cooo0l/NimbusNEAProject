#include "OLED.h"
#include <stdio.h>

OLED::OLED(uint8_t dc, uint8_t reset, uint8_t cs)
    : display(U8G2_R0, cs, dc, reset) {}

bool OLED::begin() {
    display.begin();
    display.setFont(u8g2_font_5x8_tf);
    clear();
    return true;
}

void OLED::clear() {
    display.clearBuffer();
    display.sendBuffer();
}

void OLED::firstPage() {
    display.firstPage();
}

bool OLED::nextPage() {
    return display.nextPage();
}

void OLED::drawText(int16_t x, int16_t y, const char* text) {
    display.setCursor(x, y);
    display.print(text);
}

void OLED::drawValueWithUnit(int16_t x, int16_t y, uint16_t value, const char* unit) {
    display.setCursor(x, y);
    display.print(value);
    display.print(unit);
}

void OLED::drawFloatWithUnit(int16_t x, int16_t y, float value, const char* unit,
                             uint8_t decimals, bool prependDegreeSymbol) {
    display.setCursor(x, y);
    display.print(value, decimals);
    if (prependDegreeSymbol) {
        display.print((char)176);
    }
    display.print(unit);
}

void OLED::drawFrame(int16_t x, int16_t y, uint8_t width, uint8_t height) {
    display.drawFrame(x, y, width, height);
}

void OLED::drawBox(int16_t x, int16_t y, uint8_t width, uint8_t height) {
    display.drawBox(x, y, width, height);
}

