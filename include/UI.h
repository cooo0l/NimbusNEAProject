#pragma once

#include <Arduino.h>
#include "OLED.h"
#include "SensorManager.h"

enum class Screen {
    Loading,
    MainMenu,
    SensorData,
    ExportData,
    Settings,
};

class UI {
public:
    explicit UI(OLED& display);

    void begin();
    void showLoading();
    void updateSensorData(const SensorData& data);
    void moveSelection(int delta);
    void select();
    void back();
    Screen getCurrentScreen() const;

private:
    static const uint8_t MENU_ITEM_COUNT = 3;

    void renderLoading();
    void renderMainMenu();
    void renderSensorData();
    void renderExportData();
    void renderSettings();
    void renderCurrentScreen();

    OLED& display;
    Screen currentScreen = Screen::Loading;
    SensorData currentData{};
    uint8_t selectedMenuIndex = 0;
};
