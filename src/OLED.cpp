#include "OLED.h"
#include <SPI.h>

OLED::OLED(uint8_t dc, uint8_t reset, uint8_t cs)
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, dc, reset, cs) {}

bool OLED::begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
        return false;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.display();
    return true;
}

void OLED::clear() {
    display.clearDisplay();
    display.display();
}

void OLED::clearBuffer() {
    display.clearDisplay();
}

void OLED::present() {
    display.display();
}
// Prints text
void OLED::drawText(int16_t x, int16_t y, const char* text) {
    display.setCursor(x, y);
    display.print(text);
}
// Prints readings with int value to screen
void OLED::drawValueWithUnit(int16_t x, int16_t y, uint16_t value, const char* unit) {
    display.setCursor(x, y);
    display.print(value);
    display.print(unit);
}
// Prints readings with float value to screen
void OLED::drawFloatWithUnit(int16_t x, int16_t y, float value, const char* unit,
                             uint8_t decimals, bool prependDegreeSymbol) {
    display.setCursor(x, y);
    display.print(value, decimals);
    if (prependDegreeSymbol) {
        display.write(247);
    }
    display.print(unit);
}

