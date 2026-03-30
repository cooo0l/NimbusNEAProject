#pragma once

#include <Arduino.h>
#include "OLED.h"
#include "SensorManager.h"

// Stores all the possible states of OLED
enum class Screen {
    Loading,
    MainMenu,
    SensorData,
    Settings,
};

enum class ExportFormat {
    Json,
    Csv,
};
// Decides which screen will appear
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
    ExportFormat getExportFormat() const;
    bool isLoggingEnabled() const;

private:
    static const uint8_t MENU_ITEM_COUNT = 2;
    static const uint8_t SETTINGS_ITEM_COUNT = 3;

    void renderLoading();
    void renderMainMenu();
    void renderSensorData();
    void renderSettings();
    void renderCurrentScreen();

    OLED& display;
    Screen currentScreen = Screen::Loading;
    SensorData currentData{};
    uint8_t selectedMenuIndex = 0;
    uint8_t selectedSettingsItem = 0;
    ExportFormat exportFormat = ExportFormat::Json;
    bool logReadingsEnabled = false;
    uint8_t storagePercent = 60;
};
