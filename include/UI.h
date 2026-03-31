#pragma once

#include <Arduino.h>
#include "OLED.h"
#include "SensorManager.h"

// Stores all the possible states of OLED
enum class Screen : uint8_t {
    Loading,
    MainMenu,
    SensorData,
    Settings,
};

enum class ExportFormat : uint8_t {
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
    void updateStorageStatus(bool sdAvailable, bool writeAttempted, bool lastWriteOk);
    void moveSelection(int delta);
    void select();
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
    bool logReadingsEnabled = true;
    bool sdAvailable = false;
    bool writeAttempted = false;
    bool lastWriteOk = false;
};
