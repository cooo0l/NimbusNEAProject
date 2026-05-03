#include "UI.h"
#include <stdio.h>
#include <string.h>

namespace {
const unsigned long INTERVAL_OPTIONS_MS[] = {1000UL, 2000UL, 5000UL, 10000UL, 15000UL, 30000UL};
constexpr uint8_t INTERVAL_OPTION_COUNT =
    sizeof(INTERVAL_OPTIONS_MS) / sizeof(INTERVAL_OPTIONS_MS[0]);
const unsigned long AUTO_SLEEP_OPTIONS_MS[] = {0UL, 15000UL, 30000UL, 60000UL};
constexpr uint8_t AUTO_SLEEP_OPTION_COUNT =
    sizeof(AUTO_SLEEP_OPTIONS_MS) / sizeof(AUTO_SLEEP_OPTIONS_MS[0]);
// Summarises CO2
const char* co2Summary(uint16_t co2, bool valid) {
    if (!valid) {
        return "--";
    }
    if (co2 < 800) {
        return "Good";
    }
    if (co2 < 1200) {
        return "Fair";
    }
    return "Poor";
}
// Summarises overall air quality
const char* overallAirQuality(uint16_t co2, uint16_t tvoc, bool valid) {
    if (!valid) {
        return "--";
    }
    if (co2 < 800 && tvoc < 220) {
        return "Good";
    }
    if (co2 < 1200 && tvoc < 660) {
        return "Fair";
    }
    return "Poor";
}
// Summarises TVOC
const char* tvocSummary(uint16_t tvoc, bool valid) {
    if (!valid) {
        return "--";
    }
    if (tvoc < 220) {
        return "Good";
    }
    if (tvoc < 660) {
        return "Fair";
    }
    return "Poor";
}
// Summarises temperature
const char* temperatureSummary(float temperatureC, bool valid) {
    if (!valid) {
        return "--";
    }
    if (temperatureC < 18.0f) {
        return "Low";
    }
    if (temperatureC <= 24.0f) {
        return "Normal";
    }
    return "High";
}
// Summarises humidity
const char* humiditySummary(float humidity, bool valid) {
    if (!valid) {
        return "--";
    }
    if (humidity < 30.0f) {
        return "Low";
    }
    if (humidity <= 60.0f) {
        return "Normal";
    }
    return "High";
}
}

UI::UI(OLED& display)
    : display(display) {}

void UI::begin() {
    currentScreen = Screen::MainMenu;
    renderCurrentScreen();
}

void UI::showLoading() {
    currentScreen = Screen::Loading;
    renderCurrentScreen();
}

void UI::refresh() {
    renderCurrentScreen();
}

void UI::updateSensorData(const SensorData& data) {
    currentData = data;
    if (currentScreen == Screen::SensorData) {
        renderCurrentScreen();
    }
}

void UI::updateStorageStatus(bool sdAvailable, bool writeAttempted, bool lastWriteOk,
                             uint8_t storagePercent) {
    this->sdAvailable = sdAvailable;
    this->writeAttempted = writeAttempted;
    this->lastWriteOk = lastWriteOk;
    this->storagePercent = storagePercent;
    if (currentScreen == Screen::DataAndStorage) {
        renderCurrentScreen();
    }
}
// Uses the rotary encoders delta to determine
// which menu item is selected
void UI::moveSelection(int delta) {
    if (delta == 0) {
        return;
    }

    if (currentScreen == Screen::MainMenu) {
        int nextIndex = int(selectedMenuIndex) + delta;
        while (nextIndex < 0) {
            nextIndex += MENU_ITEM_COUNT;
        }
        selectedMenuIndex = uint8_t(nextIndex % MENU_ITEM_COUNT);
        renderCurrentScreen();
        return;
    }

    if (currentScreen == Screen::Settings) {
        int nextIndex = int(selectedSettingsItem) + delta;
        while (nextIndex < 0) {
            nextIndex += SETTINGS_ITEM_COUNT;
        }
        selectedSettingsItem = uint8_t(nextIndex % SETTINGS_ITEM_COUNT);
        renderCurrentScreen();
        return;
    }

    if (currentScreen == Screen::DataAndStorage) {
        int nextIndex = int(selectedStorageItem) + delta;
        while (nextIndex < 0) {
            nextIndex += DATA_STORAGE_ITEM_COUNT;
        }
        selectedStorageItem = uint8_t(nextIndex % DATA_STORAGE_ITEM_COUNT);
        renderCurrentScreen();
        return;
    }

    if (currentScreen == Screen::DisplaySettings) {
        int nextIndex = int(selectedDisplayItem) + delta;
        while (nextIndex < 0) {
            nextIndex += DISPLAY_ITEM_COUNT;
        }
        selectedDisplayItem = uint8_t(nextIndex % DISPLAY_ITEM_COUNT);
        renderCurrentScreen();
        return;
    }

    if (currentScreen == Screen::PowerSaving) {
        int nextIndex = int(selectedPowerItem) + delta;
        while (nextIndex < 0) {
            nextIndex += POWER_ITEM_COUNT;
        }
        selectedPowerItem = uint8_t(nextIndex % POWER_ITEM_COUNT);
        renderCurrentScreen();
        return;
    }

    if (currentScreen == Screen::Units) {
        int nextIndex = int(selectedUnitsItem) + delta;
        while (nextIndex < 0) {
            nextIndex += UNITS_ITEM_COUNT;
        }
        selectedUnitsItem = uint8_t(nextIndex % UNITS_ITEM_COUNT);
        renderCurrentScreen();
        return;
    }

    if (currentScreen == Screen::ReadingInterval) {
        int nextIndex = int(selectedIntervalItem) + delta;
        while (nextIndex < 0) {
            nextIndex += INTERVAL_ITEM_COUNT;
        }
        selectedIntervalItem = uint8_t(nextIndex % INTERVAL_ITEM_COUNT);
        renderCurrentScreen();
        return;
    }
}
// Determines the next screen using the output of the rotary encoder
void UI::select() {
    if (currentScreen == Screen::MainMenu) {
        switch (selectedMenuIndex) {
        case 0:
            currentScreen = Screen::SensorData;
            break;
        case 1:
            currentScreen = Screen::Settings;
            break;
        default:
            currentScreen = Screen::MainMenu;
            break;
        }
    } else if (currentScreen == Screen::Settings) { 
        switch (selectedSettingsItem) {
        case 0:
            currentScreen = Screen::DisplaySettings;
            break;
        case 1:
            currentScreen = Screen::ReadingInterval;
            break;
        case 2:
            currentScreen = Screen::DataAndStorage;
            break;
        case 3:
            currentScreen = Screen::Units;
            break;
        case 4:
            currentScreen = Screen::PowerSaving;
            break;
        case 5:
            currentScreen = Screen::MainMenu;
            break;
        default:
            break;
        }

    } else if (currentScreen == Screen::DataAndStorage) {
        switch (selectedStorageItem) {
        case 0:
            exportFormat = exportFormat == ExportFormat::Json ? ExportFormat::Csv
                                                              : ExportFormat::Json;
            settingsChanged = true;
            break;
        case 1:
            logReadingsEnabled = !logReadingsEnabled;
            settingsChanged = true;
            break;
        case 2:
            currentScreen = Screen::Settings;
            break;
        default:
            break;
        }
    } else if (currentScreen == Screen::DisplaySettings) {
        switch (selectedDisplayItem) {
        case 0:
            displayMode = displayMode == DisplayMode::Summary ? DisplayMode::Numeric
                                                              : DisplayMode::Summary;
            settingsChanged = true;
            break;
        case 1:
            selectedSleepTimeoutIndex =
                uint8_t((selectedSleepTimeoutIndex + 1) % AUTO_SLEEP_OPTION_COUNT);
            settingsChanged = true;
            break;
        case 2:
            currentScreen = Screen::Settings;
            break;
        default:
            break;
        }

    } else if (currentScreen == Screen::PowerSaving) {
        switch (selectedPowerItem) {
        case 0:
            powerSavingEnabled = !powerSavingEnabled;
            settingsChanged = true;
            break;
        case 1:
            currentScreen = Screen::Settings;
            break;
        default:
            break;
        }

    } else if (currentScreen == Screen::ReadingInterval) {
        switch (selectedIntervalItem) {
        case 0:
            selectedIntervalIndex = uint8_t((selectedIntervalIndex + 1) % INTERVAL_OPTION_COUNT);
            settingsChanged = true;
            break;
        case 1:
            smoothingSampleCount = uint8_t((smoothingSampleCount % 8) + 1);
            settingsChanged = true;
            break;
        case 2:
            currentScreen = Screen::Settings;
            break;
        default:
            break;
        }

    } else if (currentScreen == Screen::Units) {
        switch (selectedUnitsItem) {
        case 0:
            temperatureUnit = temperatureUnit == TemperatureUnit::Celsius
                                  ? TemperatureUnit::Fahrenheit
                                  : TemperatureUnit::Celsius;
            settingsChanged = true;
            break;
        case 1:
            humidityUnit = humidityUnit == HumidityUnit::Percent
                               ? HumidityUnit::PercentRH
                               : HumidityUnit::Percent;
            settingsChanged = true;
            break;
        case 2:
            co2Unit = co2Unit == GasUnit::Ppm ? GasUnit::Ppb : GasUnit::Ppm;
            settingsChanged = true;
            break;
        case 3:
            tvocUnit = tvocUnit == GasUnit::Ppb ? GasUnit::Ppm : GasUnit::Ppb;
            settingsChanged = true;
            break;
        case 4:
            currentScreen = Screen::Settings;
            break;
        default:
            break;
        }
    } else {
        currentScreen = Screen::MainMenu;
    }

    renderCurrentScreen();
}

Screen UI::getCurrentScreen() const {
    return currentScreen;
}

ExportFormat UI::getExportFormat() const {
    return exportFormat;
}

bool UI::isLoggingEnabled() const {
    return logReadingsEnabled;
}

bool UI::isPowerSavingEnabled() const {
    return powerSavingEnabled;
}

unsigned long UI::getReadingIntervalMs() const {
    unsigned long interval = INTERVAL_OPTIONS_MS[selectedIntervalIndex];
    if (powerSavingEnabled && interval < 15000UL) {
        interval = 15000UL;
    }
    return interval;
}

unsigned long UI::getAutoSleepTimeoutMs() const {
    unsigned long timeout = AUTO_SLEEP_OPTIONS_MS[selectedSleepTimeoutIndex];
    if (powerSavingEnabled && (timeout == 0UL || timeout > 15000UL)) {
        timeout = 15000UL;
    }
    return timeout;
}

uint8_t UI::getSmoothingSampleCount() const {
    return smoothingSampleCount;
}

UISettings UI::getSettings() const {
    UISettings settings;
    settings.selectedSleepTimeoutIndex = selectedSleepTimeoutIndex;
    settings.selectedIntervalIndex = selectedIntervalIndex;
    settings.smoothingSampleCount = smoothingSampleCount;
    settings.exportFormat = exportFormat;
    settings.displayMode = displayMode;
    settings.temperatureUnit = temperatureUnit;
    settings.humidityUnit = humidityUnit;
    settings.co2Unit = co2Unit;
    settings.tvocUnit = tvocUnit;
    settings.logReadingsEnabled = logReadingsEnabled;
    settings.powerSavingEnabled = powerSavingEnabled;
    return settings;
}

void UI::applySettings(const UISettings& settings) {
    selectedSleepTimeoutIndex =
        settings.selectedSleepTimeoutIndex < AUTO_SLEEP_OPTION_COUNT
            ? settings.selectedSleepTimeoutIndex
            : 0;
    selectedIntervalIndex =
        settings.selectedIntervalIndex < INTERVAL_OPTION_COUNT
            ? settings.selectedIntervalIndex
            : 2;

    if (settings.smoothingSampleCount < 1) {
        smoothingSampleCount = 1;
    } else if (settings.smoothingSampleCount > 8) {
        smoothingSampleCount = 8;
    } else {
        smoothingSampleCount = settings.smoothingSampleCount;
    }

    exportFormat = settings.exportFormat == ExportFormat::Csv ? ExportFormat::Csv
                                                              : ExportFormat::Json;
    displayMode = settings.displayMode == DisplayMode::Numeric ? DisplayMode::Numeric
                                                               : DisplayMode::Summary;
    temperatureUnit =
        settings.temperatureUnit == TemperatureUnit::Fahrenheit
            ? TemperatureUnit::Fahrenheit
            : TemperatureUnit::Celsius;
    humidityUnit = settings.humidityUnit == HumidityUnit::PercentRH
                       ? HumidityUnit::PercentRH
                       : HumidityUnit::Percent;
    co2Unit = settings.co2Unit == GasUnit::Ppb ? GasUnit::Ppb : GasUnit::Ppm;
    tvocUnit = settings.tvocUnit == GasUnit::Ppm ? GasUnit::Ppm : GasUnit::Ppb;
    logReadingsEnabled = settings.logReadingsEnabled;
    powerSavingEnabled = settings.powerSavingEnabled;
    settingsChanged = false;
}

bool UI::consumeSettingsChanged() {
    const bool changed = settingsChanged;
    settingsChanged = false;
    return changed;
}

void UI::renderLoading() {
    display.firstPage();
    do {
        display.drawText(0, 20, "Loading...");
    } while (display.nextPage());
}

void UI::renderMainMenu() {
    display.firstPage();
    do {
        display.bigFont();
        display.drawText(10, 18, "Nimbus");
        display.smallFont();

        display.drawText(15, 44, "Air Quality");
        display.drawText(0, 52, "Monitoring System");

        display.drawFrame(6, 3, 61, 19);
        display.drawText(70, 16, selectedMenuIndex == 0 ? "> Live Data" : "  Live Data");
        display.drawText(70, 32, selectedMenuIndex == 1 ? "> Settings" : "  Settings");
    } while (display.nextPage());
}

void UI::renderSensorData() {
    display.firstPage();
    do {
        if (powerSavingEnabled || displayMode == DisplayMode::Summary) {
            display.drawFrame(41, 2, 48, 16);
            display.drawText(48, 13, "Summary");
            display.drawText(32, 30, "Overall:");
            display.drawText(80, 30,
                             overallAirQuality(currentData.CO2, currentData.TVOC,
                                               currentData.ccsValid));

            display.drawText(9, 46, "CO2:");
            display.drawText(34, 46, co2Summary(currentData.CO2, currentData.ccsValid));
            display.drawText(72, 46, "TVOC:");
            display.drawText(102, 46, tvocSummary(currentData.TVOC, currentData.ccsValid));

            display.drawText(4, 62, "Temp:");
            display.drawText(34, 62,
                             temperatureSummary(currentData.temperature, currentData.dhtValid));
            display.drawText(77, 62, "Hum:");
            display.drawText(102, 62, humiditySummary(currentData.humidity, currentData.dhtValid));
            continue;
        }
        display.drawFrame(41, 2, 48, 16);
        display.drawText(48, 13, "Numeric");

        display.drawText(5, 36, "CO2:");
        if (currentData.ccsValid) {
            if (co2Unit == GasUnit::Ppb) {
                char buffer[16];
                const uint32_t value = uint32_t(currentData.CO2) * 1000U;
                snprintf(buffer, sizeof(buffer), "%luPPB", static_cast<unsigned long>(value));
                display.drawText(26, 36, buffer);
            } else {
                display.drawValueWithUnit(26, 36, currentData.CO2, "PPM");
            }
        } else {
            display.drawText(26, 36, "--");
        }

        display.drawText(0, 52, "TVOC:");
        if (currentData.ccsValid) {
            if (tvocUnit == GasUnit::Ppm) {
                char buffer[16];
                const float value = float(currentData.TVOC) / 1000.0f;
                snprintf(buffer, sizeof(buffer), "%.2fPPM", value);
                display.drawText(26, 52, buffer);
            } else {
                display.drawValueWithUnit(26, 52, currentData.TVOC, "PPB");
            }
        } else {
            display.drawText(26, 52, "--");
        }

        display.drawText(73, 36, "HUM:");
        if (currentData.dhtValid) {
            display.drawFloatWithUnit(94, 36, currentData.humidity,
                humidityUnit == HumidityUnit::PercentRH ? "%RH" : "%",
                1);
        } else {
            display.drawText(94, 36, "--");
        }

        display.drawText(68, 52, "TEMP:");
        if (currentData.dhtValid) {
            const float tempValue =
                temperatureUnit == TemperatureUnit::Fahrenheit
                    ? (currentData.temperature * 1.8f + 32.0f)
                    : currentData.temperature;
            display.drawFloatWithUnit(
                94, 52, tempValue,
                temperatureUnit == TemperatureUnit::Fahrenheit ? "F" : "C",
                1, true);
        } else {
            display.drawText(94, 52, "--");
        }
    } while (display.nextPage());
}

void UI::renderSettings() {
    display.firstPage();
    do {
        display.drawFrame(34, 0, 62, 16);
        display.drawText(44, 12, "Settings");

        display.drawText(2, 32, selectedSettingsItem == 0 ? "> Display" : "  Display");
        display.drawText(2, 46, selectedSettingsItem == 1 ? "> Readings" : "  Readings");
        display.drawText(2, 60, selectedSettingsItem == 2 ? "> Storage" : "  Storage");
        display.drawText(68, 32, selectedSettingsItem == 3 ? "> Units" : "  Units");
        display.drawText(68, 46, selectedSettingsItem == 4 ? "> Power:" : "  Power:");
        display.drawText(110, 46, powerSavingEnabled ? "On" : "Off");
        display.drawText(68, 60, selectedSettingsItem == 5 ? "> Back" : "  Back");
        
    } while (display.nextPage());
}
// Renders the display settings screen
void UI::renderDisplaySettings() {
    char sleepText[12];
    if (AUTO_SLEEP_OPTIONS_MS[selectedSleepTimeoutIndex] == 0) {
        snprintf(sleepText, sizeof(sleepText), "Off");
    } else {
        snprintf(sleepText, sizeof(sleepText), "%lus",
                 AUTO_SLEEP_OPTIONS_MS[selectedSleepTimeoutIndex] / 1000UL);
    }

    display.firstPage();
    do {
        display.drawFrame(34, 0, 62, 16);
        display.drawText(48, 12, "Display");
        display.drawText(2, 36, selectedDisplayItem == 0 ? "> View:" : "  View:");
        display.drawText(58, 36, displayMode == DisplayMode::Summary ? "Summary" : "Numeric");
        display.drawText(2, 50, selectedDisplayItem == 1 ? "> Sleep:" : "  Sleep:");
        display.drawText(76, 50, sleepText);
        display.drawText(94, 64, selectedDisplayItem == 2 ? "> Back" : "  Back");
    } while (display.nextPage());
}
// Renders the power saving screen
void UI::renderPowerSaving() {
    char intervalText[12];
    char sleepText[12];
    // Gets the status of each setting
    snprintf(intervalText, sizeof(intervalText), "%lus", getReadingIntervalMs() / 1000UL);
    if (getAutoSleepTimeoutMs() == 0) {
        snprintf(sleepText, sizeof(sleepText), "Off");
    } else {
        snprintf(sleepText, sizeof(sleepText), "%lus", getAutoSleepTimeoutMs() / 1000UL);
    }

    display.firstPage();
    do { 
        // Creates the whole screen 
        display.drawFrame(24, 0, 80, 16);
        display.drawText(31, 12, "Power Saving");

        display.drawText(2, 28, selectedPowerItem == 0 ? "> Mode:" : "  Mode:");
        display.drawText(40, 28, powerSavingEnabled ? "On" : "Off");

        display.drawText(2, 40, "  View:");
        display.drawText(40, 40, powerSavingEnabled
                                      ? "Summary"
                                      : (displayMode == DisplayMode::Summary ? "Summary"
                                                                             : "Numeric"));

        display.drawText(2, 52, "  Sleep:");
        display.drawText(44, 52, sleepText);

        display.drawText(80, 40, "Read:");
        display.drawText(110, 40, intervalText);

        display.drawText(94, 64, selectedPowerItem == 1 ? "> Back" : "  Back");
    } while (display.nextPage());
}
// Renders the reading interval screen
void UI::renderReadingInterval() {
    display.firstPage();
    do {
        char intervalText[16];
        char smoothingText[16];
        // Formatting and copying strings into arrays intervalText and smoothingText
        snprintf(intervalText, sizeof(intervalText), "%lus",
                 INTERVAL_OPTIONS_MS[selectedIntervalIndex] / 1000UL);
        snprintf(smoothingText, sizeof(smoothingText), smoothingSampleCount == 1 ? "%u Sample": "%u Samples",
                 smoothingSampleCount);
        // Drawing all the necessary text on the screen along with title of the screen
        display.drawFrame(22, 0, 84, 16);
        display.drawText(28, 12, "Reading Control");
        display.drawText(2, 36, selectedIntervalItem == 0 ? "> Interval:" : "  Interval:");
        display.drawText(80, 36, intervalText);
        display.drawText(2, 50, selectedIntervalItem == 1 ? "> Smooth:" : "  Smooth:");
        display.drawText(76, 50, smoothingText);
        display.drawText(94, 64, selectedIntervalItem == 2 ? "> Back" : "  Back");
    } while (display.nextPage());
}
// Renders the settings page
void UI::renderUnitSettings() {
    display.firstPage();
    do {
        // Render title
        display.drawFrame(34, 0, 62, 16);
        display.drawText(50, 12, "Units");
        // Render options
        display.drawText(2, 36, selectedUnitsItem == 0 ? "> Temp:" : "  Temp:");
        display.drawText(2, 50, selectedUnitsItem == 1 ? "> Humid:" : "  Humid:");
        display.drawText(64, 36, selectedUnitsItem == 2 ? "> CO2:" : "  CO2:");
        display.drawText(64, 50, selectedUnitsItem == 3 ? "> TVOC:" : "  TVOC:");
        display.drawText(94, 64, selectedUnitsItem == 4 ? "> Back" : "  Back");
        // Render unit
        display.drawText(44, 36,
                         temperatureUnit == TemperatureUnit::Fahrenheit ? "F" : "C");
        display.drawText(44, 50,
                         humidityUnit == HumidityUnit::PercentRH ? "%RH" : "%");
        display.drawText(100, 36, co2Unit == GasUnit::Ppb ? "PPB" : "PPM");
        display.drawText(100, 50, tvocUnit == GasUnit::Ppm ? "PPM" : "PPB");
    } while (display.nextPage());
}
// Renders the data and storage page
void UI::renderDataAndStorage() {
    const uint8_t barWidth = 68;
    char storagePercentText[8];
    const uint8_t displayPercent = sdAvailable ? storagePercent : 100;
    const uint8_t fillWidth = static_cast<uint8_t>(
        (static_cast<uint16_t>(displayPercent) * barWidth) / 100U);

    if (sdAvailable) {
        snprintf(storagePercentText, sizeof(storagePercentText), "%u", storagePercent);
        const size_t length = strlen(storagePercentText);
        if (length + 1 < sizeof(storagePercentText)) {
            storagePercentText[length] = char(37);
            storagePercentText[length + 1] = '\0';
        }
    } else {
        snprintf(storagePercentText, sizeof(storagePercentText), "None");
    }

    display.firstPage();
    do {
        display.drawFrame(24, 0, 80, 16);
        display.drawText(30, 12, "Data & Storage");
        display.drawText(58, 44, storagePercentText);
        display.drawText(4, 34, "Empty");
        display.drawText(103, 34, "Full");
        display.drawFrame(31, 28, barWidth, 8);
        if (fillWidth > 0) {
            display.drawBox(31, 28, fillWidth, 8);
        }
        display.drawText(43, 25, sdAvailable ? "SD: Ready" : "SD: Missing");

        if (selectedStorageItem == 0) {
            display.drawText(0, 54, exportFormat == ExportFormat::Json ? "> Export: JSON"
                                                                        : "> Export: CSV");
        } else {
            display.drawText(0, 54, exportFormat == ExportFormat::Json ? "  Export: JSON"
                                                                        : "  Export: CSV");
        }

        if (selectedStorageItem == 1) {
            display.drawText(0, 62, logReadingsEnabled ? "> Log: On" : "> Log: Off");
        } else {
            display.drawText(0, 62, logReadingsEnabled ? "  Log: On" : "  Log: Off");
        }

        display.drawText(94, 64, selectedStorageItem == 2 ? "> Back" : "  Back");
    } while (display.nextPage());
}


void UI::renderCurrentScreen() {
    switch (currentScreen) {
    case Screen::Loading:
        renderLoading();
        break;
    case Screen::MainMenu:
        renderMainMenu();
        break;
    case Screen::SensorData:
        renderSensorData();
        break;
    case Screen::Settings:
        renderSettings();
        break;
    case Screen::DisplaySettings:
        renderDisplaySettings();
        break;
    case Screen::PowerSaving:
        renderPowerSaving();
        break;
    case Screen::ReadingInterval:
        renderReadingInterval();
        break;
    case Screen::DataAndStorage:
        renderDataAndStorage();
        break;
    case Screen::Units:
        renderUnitSettings();
    }
}
