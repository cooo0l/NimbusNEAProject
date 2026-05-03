#include "SettingsStorage.h"
#include <EEPROM.h>
// Helps confirm that EEPROM contents are settings
namespace {
constexpr uint32_t SETTINGS_MAGIC = 0x4E454131UL;
constexpr uint8_t SETTINGS_VERSION = 2;

// Exact layout that gets written into EEPROM
struct StoredSettings {
    uint32_t magic;
    uint8_t version;
    uint8_t selectedSleepTimeoutIndex;
    uint8_t selectedIntervalIndex;
    uint8_t smoothingSampleCount;
    uint8_t exportFormat;
    uint8_t displayMode;
    uint8_t temperatureUnit;
    uint8_t humidityUnit;
    uint8_t co2Unit;
    uint8_t tvocUnit;
    uint8_t logReadingsEnabled;
    uint8_t powerSavingEnabled;
};
}
// Starts EEPROM
bool SettingsStorage::begin() {
    return EEPROM.begin(EEPROM_SIZE);
}
// Loads the current UISettings
bool SettingsStorage::load(UISettings& settings) {
    StoredSettings stored{};
    EEPROM.get(0, stored);

    if (stored.magic != SETTINGS_MAGIC || stored.version != SETTINGS_VERSION) {
        return false;
    }

    settings.selectedSleepTimeoutIndex = stored.selectedSleepTimeoutIndex;
    settings.selectedIntervalIndex = stored.selectedIntervalIndex;
    settings.smoothingSampleCount = stored.smoothingSampleCount;
    settings.exportFormat = static_cast<ExportFormat>(stored.exportFormat);
    settings.displayMode = static_cast<DisplayMode>(stored.displayMode);
    settings.temperatureUnit = static_cast<TemperatureUnit>(stored.temperatureUnit);
    settings.humidityUnit = static_cast<HumidityUnit>(stored.humidityUnit);
    settings.co2Unit = static_cast<GasUnit>(stored.co2Unit);
    settings.tvocUnit = static_cast<GasUnit>(stored.tvocUnit);
    settings.logReadingsEnabled = stored.logReadingsEnabled != 0;
    return true;
}
// Saves settings to EEPROM
bool SettingsStorage::save(const UISettings& settings) {
    const StoredSettings stored{
        SETTINGS_MAGIC,
        SETTINGS_VERSION,
        settings.selectedSleepTimeoutIndex,
        settings.selectedIntervalIndex,
        settings.smoothingSampleCount,
        static_cast<uint8_t>(settings.exportFormat),
        static_cast<uint8_t>(settings.displayMode),
        static_cast<uint8_t>(settings.temperatureUnit),
        static_cast<uint8_t>(settings.humidityUnit),
        static_cast<uint8_t>(settings.co2Unit),
        static_cast<uint8_t>(settings.tvocUnit),
        static_cast<uint8_t>(settings.logReadingsEnabled ? 1 : 0),
        static_cast<uint8_t>(settings.powerSavingEnabled ? 1 : 0),
    };

    EEPROM.put(0, stored);
    return EEPROM.commit();
}
