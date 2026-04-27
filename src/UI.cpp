#include "UI.h"
#include <stdio.h>

namespace {
const unsigned long INTERVAL_OPTIONS_MS[] = {1000UL, 2000UL, 5000UL, 10000UL, 30000UL};
constexpr uint8_t INTERVAL_OPTION_COUNT =
    sizeof(INTERVAL_OPTIONS_MS) / sizeof(INTERVAL_OPTIONS_MS[0]);
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
    display.setBrightness(brightnessPercent);
    currentScreen = Screen::MainMenu;
    renderCurrentScreen();
}

void UI::showLoading() {
    currentScreen = Screen::Loading;
    renderCurrentScreen();
}

void UI::updateSensorData(const SensorData& data) {
    currentData = data;
    if (currentScreen == Screen::SensorData) {
        renderCurrentScreen();
    }
}

void UI::updateStorageStatus(bool sdAvailable, bool writeAttempted, bool lastWriteOk) {
    this->sdAvailable = sdAvailable;
    this->writeAttempted = writeAttempted;
    this->lastWriteOk = lastWriteOk;
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
        if (adjustingBrightness) {
            int nextBrightness = int(brightnessPercent) + (delta * 5);
            if (nextBrightness < 0) {
                nextBrightness = 0;
            } else if (nextBrightness > 100) {
                nextBrightness = 100;
            }
            brightnessPercent = uint8_t(nextBrightness);
            display.setBrightness(brightnessPercent);
            renderCurrentScreen();
            return;
        }

        int nextIndex = int(selectedDisplayItem) + delta;
        while (nextIndex < 0) {
            nextIndex += DISPLAY_ITEM_COUNT;
        }
        selectedDisplayItem = uint8_t(nextIndex % DISPLAY_ITEM_COUNT);
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
            break;
        case 1:
            logReadingsEnabled = !logReadingsEnabled;
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
                break;
            case 1:
            adjustingBrightness = !adjustingBrightness;
                break;
            case 2:
            adjustingBrightness = false;
                currentScreen = Screen::Settings;
                break;
            default:
                break;
        }

    } else if (currentScreen == Screen::ReadingInterval) {
        switch (selectedIntervalItem) {
        case 0:
            selectedIntervalIndex = uint8_t((selectedIntervalIndex + 1) % INTERVAL_OPTION_COUNT);
            break;
        case 1:
            smoothingSampleCount = uint8_t((smoothingSampleCount % 8) + 1);
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
            break;
        case 1:
            humidityUnit = humidityUnit == HumidityUnit::Percent
                               ? HumidityUnit::PercentRH
                               : HumidityUnit::Percent;
            break;
        case 2:
            co2Unit = co2Unit == GasUnit::Ppm ? GasUnit::Ppb : GasUnit::Ppm;
            break;
        case 3:
            tvocUnit = tvocUnit == GasUnit::Ppb ? GasUnit::Ppm : GasUnit::Ppb;
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

unsigned long UI::getReadingIntervalMs() const {
    return INTERVAL_OPTIONS_MS[selectedIntervalIndex];
}

uint8_t UI::getSmoothingSampleCount() const {
    return smoothingSampleCount;
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
        display.drawText(0, 8, "Main Menu");
        display.drawText(0, 22, selectedMenuIndex == 0 ? "> Live Data" : "  Live Data");
        display.drawText(0, 36, selectedMenuIndex == 1 ? "> Settings" : "  Settings");
    } while (display.nextPage());
}

void UI::renderSensorData() {
    display.firstPage();
    do {
        if (displayMode == DisplayMode::Summary) {
            display.drawFrame(33, 0, 64, 16);
            display.drawText(40, 12, "Summary");
            display.drawText(4, 24, "Air:");
            display.drawText(28, 24,
                             overallAirQuality(currentData.CO2, currentData.TVOC,
                                               currentData.ccsValid));

            display.drawText(4, 38, "CO2:");
            display.drawText(34, 38, co2Summary(currentData.CO2, currentData.ccsValid));
            display.drawText(72, 38, "TVOC:");
            display.drawText(102, 38, tvocSummary(currentData.TVOC, currentData.ccsValid));

            display.drawText(4, 54, "Temp:");
            display.drawText(34, 54,
                             temperatureSummary(currentData.temperature, currentData.dhtValid));
            display.drawText(72, 54, "Hum:");
            display.drawText(96, 54, humiditySummary(currentData.humidity, currentData.dhtValid));
            continue;
        }

        display.drawText(10, 10, "CO2:");
        if (currentData.ccsValid) {
            if (co2Unit == GasUnit::Ppb) {
                char buffer[16];
                const uint32_t value = uint32_t(currentData.CO2) * 1000U;
                snprintf(buffer, sizeof(buffer), "%luPPB", static_cast<unsigned long>(value));
                display.drawText(40, 10, buffer);
            } else {
                display.drawValueWithUnit(40, 10, currentData.CO2, "PPM");
            }
        } else {
            display.drawText(40, 10, "--");
        }

        display.drawText(10, 24, "TVOC:");
        if (currentData.ccsValid) {
            if (tvocUnit == GasUnit::Ppm) {
                char buffer[16];
                const float value = float(currentData.TVOC) / 1000.0f;
                snprintf(buffer, sizeof(buffer), "%.2fPPM", value);
                display.drawText(40, 24, buffer);
            } else {
                display.drawValueWithUnit(40, 24, currentData.TVOC, "PPB");
            }
        } else {
            display.drawText(40, 24, "--");
        }

        display.drawText(10, 38, "HUM:");
        if (currentData.dhtValid) {
            display.drawFloatWithUnit(40, 38, currentData.humidity,
                humidityUnit == HumidityUnit::PercentRH ? "%RH" : "%",
                1);
        } else {
            display.drawText(40, 38, "--");
        }

        display.drawText(10, 52, "TEMP:");
        if (currentData.dhtValid) {
            const float tempValue =
                temperatureUnit == TemperatureUnit::Fahrenheit
                    ? (currentData.temperature * 1.8f + 32.0f)
                    : currentData.temperature;
            display.drawFloatWithUnit(
                40, 52, tempValue,
                temperatureUnit == TemperatureUnit::Fahrenheit ? "F" : "C",
                1, true);
        } else {
            display.drawText(40, 52, "--");
        }
    } while (display.nextPage());
}

void UI::renderSettings() {
    display.firstPage();
    do {
        display.drawFrame(34, 0, 62, 16);
        display.drawText(44, 12, "Settings");

        display.drawText(2, 36, selectedSettingsItem == 0 ? "> Display" : "  Display");
        display.drawText(2, 50, selectedSettingsItem == 1 ? "> Readings" : "  Readings");
        display.drawText(2, 64, selectedSettingsItem == 2 ? "> Storage" : "  Storage");
        display.drawText(58, 36, selectedSettingsItem == 3 ? "> Units" : "  Units");
        display.drawText(58, 50, selectedSettingsItem == 4 ? "> Power Saving" : "  Power Saving");
        display.drawText(58, 64, selectedSettingsItem == 5 ? "> Back" : "  Back");
        
    } while (display.nextPage());
}
// Renders the display settings screen
void UI::renderDisplaySettings() {
    // Render scrollable bar
    char brightnessText[10];
    const uint8_t barWidth = 52;
    const uint8_t fillWidth = uint8_t(
        (uint16_t(brightnessPercent) * barWidth) / 100U);

    snprintf(brightnessText, sizeof(brightnessText), "%u%%", brightnessPercent);

    display.firstPage();
    do {
        // Render title
        display.drawFrame(34, 0, 62, 16);
        display.drawText(48, 12, "Display");
        // Render options
        display.drawText(2, 36, selectedDisplayItem == 0 ? "> View:" : "  View:");
        display.drawText(58, 36, displayMode == DisplayMode::Summary ? "Summary" : "Numeric");

        display.drawText(2, 50, selectedDisplayItem == 1 ? "> Bright:" : "  Bright:");
        display.drawFrame(58, 42, barWidth, 10);
        // Fills box based on brightness selected
        if (fillWidth > 0) { 
            display.drawBox(58, 42, fillWidth, 10);
        }
        display.drawText(112, 50, brightnessText);
        
        if (adjustingBrightness) {
            display.drawText(32, 64, "Press to finish");
        } else {
            display.drawText(94, 64, selectedDisplayItem == 2 ? "> Back" : "  Back");
        }
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
    const uint8_t fillWidth = 40;

    display.firstPage();
    do {
        display.drawFrame(24, 0, 80, 16);
        display.drawText(30, 12, "Data & Storage");
        display.drawText(58, 42, "60%");
        display.drawText(4, 34, "Empty");
        display.drawText(103, 34, "Full");
        display.drawFrame(30, 28, barWidth, 8);
        display.drawBox(30, 28, fillWidth, 8);
        display.drawText(44, 24, sdAvailable ? "SD: Ready" : "SD: Missing");

        if (selectedStorageItem == 0) {
            display.drawText(0, 50, exportFormat == ExportFormat::Json ? "> Export: JSON"
                                                                        : "> Export: CSV");
        } else {
            display.drawText(0, 50, exportFormat == ExportFormat::Json ? "  Export: JSON"
                                                                        : "  Export: CSV");
        }

        if (selectedStorageItem == 1) {
            display.drawText(0, 58, logReadingsEnabled ? "> Log: On" : "> Log: Off");
        } else {
            display.drawText(0, 58, logReadingsEnabled ? "  Log: On" : "  Log: Off");
        }

        if (!writeAttempted) {
            display.drawText(72, 58, "W:--");
        } else {
            display.drawText(72, 58, lastWriteOk ? "W:OK" : "W:FAIL");
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
