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

void OLED::showLoading() {
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("Loading...");
    display.display();
}

void OLED::showSensorData(const SensorData& data) {
    display.clearDisplay();

    display.setCursor(10, 10);
    display.println("CO2:");
    display.setCursor(40, 10);
    if (data.ccsValid) {
        display.print(data.CO2);
        display.println("PPM");
    } else {
        display.println("--");
    }

    display.setCursor(10, 24);
    display.println("TVOC:");
    display.setCursor(40, 24);
    if (data.ccsValid) {
        display.print(data.TVOC);
        display.println("PPB");
    } else {
        display.println("--");
    }

    display.setCursor(10, 38);
    display.println("HUM:");
    display.setCursor(40, 38);
    if (data.dhtValid) {
        display.print(data.humidity);
        display.println("%");
    } else {
        display.println("--");
    }

    display.setCursor(10, 52);
    display.println("TEMP:");
    display.setCursor(40, 52);
    if (data.dhtValid) {
        display.print(data.temperature);
        display.print(char(247));
        display.println("C");
    } else {
        display.println("--");
    }

    display.display();
}
