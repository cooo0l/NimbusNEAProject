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

void UI::moveSelection(int delta) {
    if (currentScreen != Screen::MainMenu || delta == 0) {
        return;
    }

    int nextIndex = static_cast<int>(selectedMenuIndex) + delta;
    while (nextIndex < 0) {
        nextIndex += MENU_ITEM_COUNT;
    }
    selectedMenuIndex = static_cast<uint8_t>(nextIndex % MENU_ITEM_COUNT);
    renderCurrentScreen();
}

void UI::select() {
    if (currentScreen == Screen::MainMenu) {
        switch (selectedMenuIndex) {
        case 0:
            currentScreen = Screen::SensorData;
            break;
        case 1:
            currentScreen = Screen::ExportData;
            break;
        case 2:
            currentScreen = Screen::Settings;
            break;
        default:
            currentScreen = Screen::MainMenu;
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

void UI::renderLoading() {
    display.clearBuffer();
    display.drawText(0, 20, "Loading...");
    display.present();
}

void UI::renderMainMenu() {
    static const char* const menuItems[MENU_ITEM_COUNT] = {
        "Live Data",
        "Export Data",
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

void UI::renderExportData() {
    display.clearBuffer();
    display.drawText(0, 8, "Export Data");
    display.drawText(0, 28, "Use Serial");
    display.drawText(0, 42, "Press to return");
    display.present();
}

void UI::renderSettings() {
    display.clearBuffer();
    display.drawText(0, 8, "Settings");
    display.drawText(0, 28, "Coming soon");
    display.drawText(0, 42, "Press to return");
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
    case Screen::ExportData:
        renderExportData();
        break;
    case Screen::Settings:
        renderSettings();
        break;
    }
}
