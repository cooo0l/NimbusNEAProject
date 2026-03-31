#include "UI.h"

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

void UI::updateStorageStatus(bool sdAvailable, bool writeAttempted, bool lastWriteOk) {
    this->sdAvailable = sdAvailable;
    this->writeAttempted = writeAttempted;
    this->lastWriteOk = lastWriteOk;
    if (currentScreen == Screen::Settings) {
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
    } while (display.nextPage());
}

void UI::renderSettings() {
    const uint8_t barWidth = 68;
    const uint8_t fillWidth = 40;

    display.firstPage();
    do {
        display.drawFrame(24, 0, 80, 16);
        display.drawText(31, 12, "Data & Storage");
        display.drawText(54, 24, "60%");
        display.drawText(4, 34, "Empty");
        display.drawText(103, 34, "Full");
        display.drawFrame(30, 28, barWidth, 8);
        display.drawBox(30, 28, fillWidth, 8);
        display.drawText(0, 42, sdAvailable ? "SD: Ready" : "SD: Missing");

        if (selectedSettingsItem == 0) {
            display.drawText(0, 50, exportFormat == ExportFormat::Json ? "> Export: JSON"
                                                                        : "> Export: CSV");
        } else {
            display.drawText(0, 50, exportFormat == ExportFormat::Json ? "  Export: JSON"
                                                                        : "  Export: CSV");
        }

        if (selectedSettingsItem == 1) {
            display.drawText(0, 58, logReadingsEnabled ? "> Log: On" : "> Log: Off");
        } else {
            display.drawText(0, 58, logReadingsEnabled ? "  Log: On" : "  Log: Off");
        }

        if (!writeAttempted) {
            display.drawText(72, 58, "W:--");
        } else {
            display.drawText(72, 58, lastWriteOk ? "W:OK" : "W:FAIL");
        }
        display.drawText(0, 64, selectedSettingsItem == 2 ? "> Back" : "  Back");
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
    }
}
