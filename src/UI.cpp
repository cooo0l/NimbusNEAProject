#include "UI.h"
#include <stdio.h>

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

void UI::updateSensorData(const SensorData& data) {
    currentData = data;
    if (currentScreen == Screen::SensorData) {
        renderCurrentScreen();
    }
}

void UI::moveSelection(int delta) {
    if (delta == 0) {
        return;
    }

    if (currentScreen == Screen::MainMenu) {
        int nextIndex = static_cast<int>(selectedMenuIndex) + delta;
        while (nextIndex < 0) {
            nextIndex += MENU_ITEM_COUNT;
        }
        selectedMenuIndex = static_cast<uint8_t>(nextIndex % MENU_ITEM_COUNT);
        renderCurrentScreen();
        return;
    }

    if (currentScreen == Screen::Settings) {
        int nextIndex = static_cast<int>(selectedSettingsItem) + delta;
        while (nextIndex < 0) {
            nextIndex += SETTINGS_ITEM_COUNT;
        }
        selectedSettingsItem = static_cast<uint8_t>(nextIndex % SETTINGS_ITEM_COUNT);
        renderCurrentScreen();
    }
}

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
            exportFormat = exportFormat == ExportFormat::Json ? ExportFormat::Csv
                                                              : ExportFormat::Json;
            break;
        case 1:
            logReadingsEnabled = !logReadingsEnabled;
            break;
        case 2:
            currentScreen = Screen::MainMenu;
            break;
        default:
            break;
        }
    } else {
        currentScreen = Screen::MainMenu;
    }

    renderCurrentScreen();
}

void UI::back() {
    if (currentScreen != Screen::MainMenu) {
        currentScreen = Screen::MainMenu;
        renderCurrentScreen();
    }
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

void UI::renderLoading() {
    display.clearBuffer();
    display.drawText(0, 20, "Loading...");
    display.present();
}

void UI::renderMainMenu() {
    static const char* const menuItems[MENU_ITEM_COUNT] = {
        "Live Data",
        "Settings",
    };

    display.clearBuffer();
    display.drawText(0, 8, "Main Menu");

    for (uint8_t i = 0; i < MENU_ITEM_COUNT; ++i) {
        const int16_t y = 22 + (i * 14);
        display.drawText(0, y, i == selectedMenuIndex ? ">" : " ");
        display.drawText(10, y, menuItems[i]);
    }

    display.present();
}

void UI::renderSensorData() {
    display.clearBuffer();

    display.drawText(10, 10, "CO2:");
    if (currentData.ccsValid) {
        display.drawValueWithUnit(40, 10, currentData.CO2, "PPM");
    } else {
        display.drawText(40, 10, "--");
    }

    display.drawText(10, 24, "TVOC:");
    if (currentData.ccsValid) {
        display.drawValueWithUnit(40, 24, currentData.TVOC, "PPB");
    } else {
        display.drawText(40, 24, "--");
    }

    display.drawText(10, 38, "HUM:");
    if (currentData.dhtValid) {
        display.drawFloatWithUnit(40, 38, currentData.humidity, "%", 1);
    } else {
        display.drawText(40, 38, "--");
    }

    display.drawText(10, 52, "TEMP:");
    if (currentData.dhtValid) {
        display.drawFloatWithUnit(40, 52, currentData.temperature, "C", 1, true);
    } else {
        display.drawText(40, 52, "--");
    }

    display.present();
}

void UI::renderSettings() {
    char percentText[8];
    snprintf(percentText, sizeof(percentText), "%u%%", storagePercent);

    const uint8_t barWidth = 68;
    const uint8_t fillWidth = static_cast<uint8_t>((static_cast<uint16_t>(barWidth) *
                                                    storagePercent) / 100);

    display.clearBuffer();

    display.drawFrame(24, 0, 80, 16);
    display.drawText(31, 12, "Data & Storage");
    display.drawText(54, 24, percentText);
    display.drawText(4, 34, "Empty");
    display.drawText(103, 34, "Full");
    display.drawFrame(30, 28, barWidth, 8);
    display.drawBox(30, 28, fillWidth, 8);

    display.drawText(selectedSettingsItem == 0 ? 0 : 4, 46,
                     selectedSettingsItem == 0 ? ">" : " ");
    display.drawText(8, 46, "Export:");
    display.drawText(48, 46, exportFormat == ExportFormat::Json ? "JSON" : "CSV");

    display.drawText(selectedSettingsItem == 1 ? 0 : 4, 56,
                     selectedSettingsItem == 1 ? ">" : " ");
    display.drawText(8, 56, "Log:");
    display.drawText(48, 56, logReadingsEnabled ? "On" : "Off");

    display.drawText(selectedSettingsItem == 2 ? 0 : 4, 63,
                     selectedSettingsItem == 2 ? ">" : " ");
    display.drawText(8, 63, "Back");

    display.present();
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
    }
}
