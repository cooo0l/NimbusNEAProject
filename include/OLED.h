#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SensorManager.h"

class OLED {
public:
    OLED(uint8_t dc, uint8_t reset, uint8_t cs);

    bool begin();
    void clear();
    void showLoading();
    void showSensorData(const SensorData& data);

private:
    static const uint8_t SCREEN_WIDTH = 128;
    static const uint8_t SCREEN_HEIGHT = 64;

    Adafruit_SSD1306 display;
};
