#pragma once

#include <Arduino.h>
#include "UI.h"
// Responsible for startup, loading data and saving data for EEPROM
class SettingsStorage {
public:
    bool begin();
    bool load(UISettings& settings);
    bool save(const UISettings& settings);

private:
    static const size_t EEPROM_SIZE = 64;
};
