#include "OLED.h"
#include <stdio.h>
#include <string.h>

OLED::OLED(uint8_t dc, uint8_t reset, uint8_t cs)
    : display(U8G2_R0, cs, dc, reset) {}

bool OLED::begin() {
    display.begin();
    display.setFont(u8g2_font_5x8_tf);
    clear();
    return true;
}

void OLED::clear() {
    commandCount = 0;
    present();
}

void OLED::clearBuffer() {
    commandCount = 0;
}

void OLED::present() {
    display.firstPage();
    do {
        for (uint8_t i = 0; i < commandCount; ++i) {
            switch (commands[i].type) {
            case CommandType::Text:
                display.setCursor(commands[i].x, commands[i].y);
                display.print(commands[i].text);
                break;
            case CommandType::Frame:
                display.drawFrame(commands[i].x, commands[i].y, commands[i].width,
                                  commands[i].height);
                break;
            case CommandType::Box:
                display.drawBox(commands[i].x, commands[i].y, commands[i].width,
                                commands[i].height);
                break;
            }
        }
    } while (display.nextPage());
}

void OLED::drawText(int16_t x, int16_t y, const char* text) {
    addCommand(x, y, text);
}

void OLED::drawValueWithUnit(int16_t x, int16_t y, uint16_t value, const char* unit) {
    char buffer[MAX_TEXT_LENGTH];
    snprintf(buffer, sizeof(buffer), "%u%s", value, unit);
    addCommand(x, y, buffer);
}

void OLED::drawFloatWithUnit(int16_t x, int16_t y, float value, const char* unit,
                             uint8_t decimals, bool prependDegreeSymbol) {
    char valueBuffer[12];
    dtostrf(value, 0, decimals, valueBuffer);

    char buffer[MAX_TEXT_LENGTH];
    if (prependDegreeSymbol) {
        snprintf(buffer, sizeof(buffer), "%s%c%s", valueBuffer, 176, unit);
    } else {
        snprintf(buffer, sizeof(buffer), "%s%s", valueBuffer, unit);
    }

    addCommand(x, y, buffer);
}

void OLED::drawFrame(int16_t x, int16_t y, uint8_t width, uint8_t height) {
    addCommand(CommandType::Frame, x, y, width, height);
}

void OLED::drawBox(int16_t x, int16_t y, uint8_t width, uint8_t height) {
    addCommand(CommandType::Box, x, y, width, height);
}

bool OLED::addCommand(int16_t x, int16_t y, const char* text) {
    if (commandCount >= MAX_DRAW_COMMANDS) {
        return false;
    }

    commands[commandCount].type = CommandType::Text;
    commands[commandCount].x = x;
    commands[commandCount].y = y;
    strncpy(commands[commandCount].text, text, MAX_TEXT_LENGTH - 1);
    commands[commandCount].text[MAX_TEXT_LENGTH - 1] = '\0';
    ++commandCount;
    return true;
}

bool OLED::addCommand(CommandType type, int16_t x, int16_t y, uint8_t width, uint8_t height) {
    if (commandCount >= MAX_DRAW_COMMANDS) {
        return false;
    }

    commands[commandCount].type = type;
    commands[commandCount].x = x;
    commands[commandCount].y = y;
    commands[commandCount].width = width;
    commands[commandCount].height = height;
    commands[commandCount].text[0] = '\0';
    ++commandCount;
    return true;
}

