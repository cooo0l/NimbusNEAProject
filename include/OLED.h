#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

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
    void drawFrame(int16_t x, int16_t y, uint8_t width, uint8_t height);
    void drawBox(int16_t x, int16_t y, uint8_t width, uint8_t height);

private:
    enum class CommandType : uint8_t {
        Text,
        Frame,
        Box,
    };

    static const uint8_t MAX_DRAW_COMMANDS = 16;
    static const uint8_t MAX_TEXT_LENGTH = 16;

    struct DrawCommand {
        CommandType type;
        uint8_t x;
        uint8_t y;
        uint8_t width = 0;
        uint8_t height = 0;
        char text[MAX_TEXT_LENGTH];
    };

    bool addCommand(int16_t x, int16_t y, const char* text);
    bool addCommand(CommandType type, int16_t x, int16_t y, uint8_t width, uint8_t height);

    U8G2_SSD1306_128X64_NONAME_1_4W_HW_SPI display;
    DrawCommand commands[MAX_DRAW_COMMANDS]{};
    uint8_t commandCount = 0;
};
