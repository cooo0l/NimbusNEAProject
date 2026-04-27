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
    DisplaySettings,
    ReadingInterval,
    DataAndStorage,
    Units
};

enum class ExportFormat : uint8_t {
    Json,
    Csv,
};

enum class TemperatureUnit : uint8_t {
    Celsius,
    Fahrenheit,
};

enum class HumidityUnit : uint8_t {
    Percent,
    PercentRH,
};

enum class GasUnit : uint8_t {
    Ppm,
    Ppb,
};

enum class DisplayMode : uint8_t {
    Summary,
    Numeric,
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
    unsigned long getReadingIntervalMs() const;
    uint8_t getSmoothingSampleCount() const;

private:
    static const uint8_t MENU_ITEM_COUNT = 2;
    static const uint8_t SETTINGS_ITEM_COUNT = 6;
    static const uint8_t DISPLAY_ITEM_COUNT = 3;
    static const uint8_t INTERVAL_ITEM_COUNT = 3;
    static const uint8_t UNITS_ITEM_COUNT = 5;
    static const uint8_t DATA_STORAGE_ITEM_COUNT = 3;

    void renderLoading();
    void renderMainMenu();
    void renderSensorData();
    void renderSettings();
    void renderDisplaySettings();
    void renderReadingInterval();
    void renderDataAndStorage();
    void renderUnitSettings();

    void renderCurrentScreen();

    OLED& display;
    Screen currentScreen = Screen::Loading;
    SensorData currentData{};
    uint8_t selectedMenuIndex = 0;
    uint8_t selectedSettingsItem = 0;
    uint8_t selectedStorageItem = 0;
    uint8_t selectedDisplayItem = 0;
    uint8_t selectedIntervalItem = 0;
    uint8_t selectedUnitsItem = 0;
    uint8_t selectedIntervalIndex = 2;
    uint8_t smoothingSampleCount = 5;
    uint8_t brightnessPercent = 75;
    ExportFormat exportFormat = ExportFormat::Json;
    DisplayMode displayMode = DisplayMode::Summary;
    TemperatureUnit temperatureUnit = TemperatureUnit::Celsius;
    HumidityUnit humidityUnit = HumidityUnit::Percent;
    GasUnit co2Unit = GasUnit::Ppm;
    GasUnit tvocUnit = GasUnit::Ppb;
    bool adjustingBrightness = false;
    bool logReadingsEnabled = true;
    bool sdAvailable = false;
    bool writeAttempted = false;
    bool lastWriteOk = false;
};
