/*
 * Settings Management Implementation for Bobcat Ignition Controller
 * Single source of truth persisted in NVS ("bobcat" namespace).
 */

#include "settings.h"
#include "config.h"
#include <esp32-hal-log.h>
#include <string.h>

SettingsManager g_settingsManager;

SettingsManager::SettingsManager() {
    setDefaultSettings();
}

bool SettingsManager::begin() {
    if (!prefs.begin("bobcat", false)) {
        Serial.println("ERROR: Failed to initialize settings storage");
        return false;
    }

    if (!loadSettings()) {
        Serial.println("WARNING: Failed to load settings, using defaults");
        setDefaultSettings();
        saveSettings();
    }

    Serial.println("Settings Manager initialized successfully");
    printCurrentSettings();
    return true;
}

bool SettingsManager::loadSettings() {
    size_t expectedSize = sizeof(BobcatSettings);
    size_t actualSize = prefs.getBytesLength("settings");

    if (actualSize != expectedSize) {
        Serial.printf("Settings size mismatch: expected %u, got %u\n",
                      (unsigned)expectedSize, (unsigned)actualSize);
        return false;
    }

    size_t bytesRead = prefs.getBytes("settings", &currentSettings, expectedSize);
    if (bytesRead != expectedSize) {
        Serial.println("Failed to read settings from storage");
        return false;
    }

    if (currentSettings.settingsVersion != SETTINGS_VERSION) {
        Serial.printf("Settings version mismatch: expected %u, got %u\n",
                      (unsigned)SETTINGS_VERSION, (unsigned)currentSettings.settingsVersion);
        return false;
    }

    if (!validateChecksum(currentSettings)) {
        Serial.println("Settings checksum validation failed - data may be corrupt");
        return false;
    }

    if (!validateSettings(currentSettings)) {
        Serial.println("Settings validation failed - using defaults");
        return false;
    }

    Serial.println("Settings loaded successfully from storage");
    return true;
}

bool SettingsManager::saveSettings() {
    currentSettings.settingsVersion = SETTINGS_VERSION;
    currentSettings.checksum = calculateChecksum(currentSettings);

    size_t bytesWritten = prefs.putBytes("settings", &currentSettings, sizeof(BobcatSettings));
    if (bytesWritten != sizeof(BobcatSettings)) {
        Serial.println("ERROR: Failed to save settings to storage");
        return false;
    }

    Serial.println("Settings saved successfully to storage");
    return true;
}

bool SettingsManager::resetToDefaults() {
    Serial.println("Resetting settings to factory defaults");
    setDefaultSettings();
    return saveSettings();
}

void SettingsManager::setDefaultPinMap() {
    for (int i = 0; i < PIN_FUNC_COUNT; i++) {
        currentSettings.pinGpio[i] = PIN_TABLE[i].defaultGpio;
    }
}

void SettingsManager::setDefaultSettings() {
    // Engine timing
    currentSettings.glowPlugDuration = GLOW_PLUG_DURATION;
    currentSettings.crankingTimeout = IGNITION_TIMEOUT;
    currentSettings.cooldownDuration = COOLDOWN_DURATION;
    currentSettings.glowAssistDuration = 5000;   // 5 s glow assist during crank

    // Thresholds
    currentSettings.maxCoolantTemp = MAX_COOLANT_TEMP;
    currentSettings.minOilPressure = MIN_OIL_PRESSURE;
    currentSettings.minHydPressure = MIN_HYD_PRESSURE;
    currentSettings.minBatteryVoltage = MIN_BATTERY_VOLTAGE;
    currentSettings.maxBatteryVoltage = MAX_BATTERY_VOLTAGE;

    // WiFi: no hard-coded station secret. Station disabled until configured.
    currentSettings.wifiSSID[0] = '\0';
    currentSettings.wifiPassword[0] = '\0';
    strncpy(currentSettings.apSSID, "Bobcat-743", sizeof(currentSettings.apSSID) - 1);
    currentSettings.apSSID[sizeof(currentSettings.apSSID) - 1] = '\0';
    strncpy(currentSettings.apPassword, "bobcat743", sizeof(currentSettings.apPassword) - 1);
    currentSettings.apPassword[sizeof(currentSettings.apPassword) - 1] = '\0';

    // Calibration (unified)
    currentSettings.tempSensorScale = TEMP_SENSOR_SCALE;
    currentSettings.oilPressureScale = OIL_PRESSURE_SCALE;
    currentSettings.hydPressureScale = HYD_PRESSURE_SCALE;
    currentSettings.batteryDivider = BATTERY_VOLTAGE_DIVIDER;
    currentSettings.fuelLevelEmpty = (uint16_t)FUEL_LEVEL_EMPTY;
    currentSettings.fuelLevelFull = (uint16_t)FUEL_LEVEL_FULL;
    currentSettings.fuelLevelLowThreshold = 15;

    // Pin map
    setDefaultPinMap();

    // Engine hours
    currentSettings.engineHours = 0.0f;

    // MQTT stub
    currentSettings.mqttEnabled = 0;
    strncpy(currentSettings.mqttHost, "", sizeof(currentSettings.mqttHost) - 1);
    currentSettings.mqttHost[0] = '\0';
    currentSettings.mqttPort = 1883;
    strncpy(currentSettings.mqttTopic, "bobcat/status", sizeof(currentSettings.mqttTopic) - 1);
    currentSettings.mqttTopic[sizeof(currentSettings.mqttTopic) - 1] = '\0';

    // Time stub
    currentSettings.ntpEnabled = 0;
    currentSettings.utcOffset = 0;

    currentSettings.settingsVersion = SETTINGS_VERSION;
    currentSettings.checksum = 0;
}

uint8_t SettingsManager::getPinGpio(int func) const {
    if (func < 0 || func >= PIN_FUNC_COUNT) return 0;
    return currentSettings.pinGpio[func];
}

bool SettingsManager::setGlowAssist(uint32_t assistMs) {
    uint32_t sec = assistMs / 1000;
    if (sec > SettingsLimits::MAX_GLOW_ASSIST) {
        Serial.printf("Invalid glow assist: %u s\n", (unsigned)sec); return false;
    }
    currentSettings.glowAssistDuration = assistMs;
    return true;
}

bool SettingsManager::updateEngineSettings(uint32_t glowMs, uint32_t crankMs, uint32_t cooldownMs) {
    uint32_t glowS = glowMs / 1000, crankS = crankMs / 1000, cooldownS = cooldownMs / 1000;
    if (glowS < SettingsLimits::MIN_GLOW_DURATION || glowS > SettingsLimits::MAX_GLOW_DURATION) {
        Serial.printf("Invalid glow duration: %u s\n", (unsigned)glowS); return false;
    }
    if (crankS < SettingsLimits::MIN_CRANKING_TIMEOUT || crankS > SettingsLimits::MAX_CRANKING_TIMEOUT) {
        Serial.printf("Invalid cranking timeout: %u s\n", (unsigned)crankS); return false;
    }
    if (cooldownS < SettingsLimits::MIN_COOLDOWN_DURATION || cooldownS > SettingsLimits::MAX_COOLDOWN_DURATION) {
        Serial.printf("Invalid cooldown: %u s\n", (unsigned)cooldownS); return false;
    }
    currentSettings.glowPlugDuration = glowMs;
    currentSettings.crankingTimeout = crankMs;
    currentSettings.cooldownDuration = cooldownMs;
    return true;
}

bool SettingsManager::updateAlarmThresholds(int16_t maxTemp, int16_t minOilPsi, float minVolt, float maxVolt) {
    if (maxTemp < SettingsLimits::MIN_COOLANT_TEMP || maxTemp > SettingsLimits::MAX_COOLANT_TEMP) return false;
    if (minOilPsi < SettingsLimits::MIN_OIL_PRESSURE_LIMIT || minOilPsi > SettingsLimits::MAX_OIL_PRESSURE_LIMIT) return false;
    if (minVolt < SettingsLimits::MIN_BATTERY_VOLTAGE_LIMIT || minVolt > SettingsLimits::MAX_BATTERY_VOLTAGE_LIMIT) return false;
    if (maxVolt < SettingsLimits::MIN_BATTERY_VOLTAGE_LIMIT || maxVolt > SettingsLimits::MAX_BATTERY_VOLTAGE_LIMIT || maxVolt <= minVolt) return false;
    currentSettings.maxCoolantTemp = maxTemp;
    currentSettings.minOilPressure = minOilPsi;
    currentSettings.minBatteryVoltage = minVolt;
    currentSettings.maxBatteryVoltage = maxVolt;
    return true;
}

bool SettingsManager::updateHydraulicThreshold(int16_t minHydPsi) {
    if (minHydPsi < SettingsLimits::MIN_HYD_PRESSURE_LIMIT || minHydPsi > SettingsLimits::MAX_HYD_PRESSURE_LIMIT) return false;
    currentSettings.minHydPressure = minHydPsi;
    return true;
}

bool SettingsManager::updateWifiSettings(const char* ssid, const char* password) {
    // Empty SSID is allowed => station disabled.
    if (!ssid) return false;
    if (strlen(ssid) > SettingsLimits::MAX_SSID_LENGTH) return false;
    if (password && strlen(password) > 0 &&
        (strlen(password) < SettingsLimits::MIN_PASSWORD_LENGTH ||
         strlen(password) > SettingsLimits::MAX_PASSWORD_LENGTH)) return false;

    strncpy(currentSettings.wifiSSID, ssid, sizeof(currentSettings.wifiSSID) - 1);
    currentSettings.wifiSSID[sizeof(currentSettings.wifiSSID) - 1] = '\0';
    if (password) {
        strncpy(currentSettings.wifiPassword, password, sizeof(currentSettings.wifiPassword) - 1);
        currentSettings.wifiPassword[sizeof(currentSettings.wifiPassword) - 1] = '\0';
    }
    return true;
}

bool SettingsManager::updateApSettings(const char* ssid, const char* password) {
    if (!ssid || strlen(ssid) < SettingsLimits::MIN_AP_SSID_LENGTH || strlen(ssid) > SettingsLimits::MAX_SSID_LENGTH) return false;
    if (password && strlen(password) > 0 &&
        (strlen(password) < SettingsLimits::MIN_PASSWORD_LENGTH ||
         strlen(password) > SettingsLimits::MAX_PASSWORD_LENGTH)) return false;
    strncpy(currentSettings.apSSID, ssid, sizeof(currentSettings.apSSID) - 1);
    currentSettings.apSSID[sizeof(currentSettings.apSSID) - 1] = '\0';
    if (password) {
        strncpy(currentSettings.apPassword, password, sizeof(currentSettings.apPassword) - 1);
        currentSettings.apPassword[sizeof(currentSettings.apPassword) - 1] = '\0';
    }
    return true;
}

bool SettingsManager::setTempScale(float v) {
    if (v < SettingsLimits::MIN_TEMP_SCALE || v > SettingsLimits::MAX_TEMP_SCALE) return false;
    currentSettings.tempSensorScale = v; return true;
}
bool SettingsManager::setOilScale(float v) {
    if (v < SettingsLimits::MIN_PRESSURE_SCALE || v > SettingsLimits::MAX_PRESSURE_SCALE) return false;
    currentSettings.oilPressureScale = v; return true;
}
bool SettingsManager::setHydScale(float v) {
    if (v < SettingsLimits::MIN_PRESSURE_SCALE || v > SettingsLimits::MAX_PRESSURE_SCALE) return false;
    currentSettings.hydPressureScale = v; return true;
}
bool SettingsManager::setBatteryDivider(float v) {
    if (v < SettingsLimits::MIN_BATTERY_DIVIDER || v > SettingsLimits::MAX_BATTERY_DIVIDER) return false;
    currentSettings.batteryDivider = v; return true;
}
bool SettingsManager::setFuelEmpty(uint16_t v) {
    if (v > SettingsLimits::MAX_FUEL_ADC || v >= currentSettings.fuelLevelFull) return false;
    currentSettings.fuelLevelEmpty = v; return true;
}
bool SettingsManager::setFuelFull(uint16_t v) {
    if (v > SettingsLimits::MAX_FUEL_ADC || v <= currentSettings.fuelLevelEmpty) return false;
    currentSettings.fuelLevelFull = v; return true;
}
bool SettingsManager::setFuelLowThreshold(uint8_t v) {
    if (v > 100) return false;
    currentSettings.fuelLevelLowThreshold = v; return true;
}

bool SettingsManager::updateCalibration(float tempScale, float oilScale, float hydScale,
                                        float batteryDivider, uint16_t fuelEmpty, uint16_t fuelFull,
                                        uint8_t fuelLowThreshold) {
    if (tempScale < SettingsLimits::MIN_TEMP_SCALE || tempScale > SettingsLimits::MAX_TEMP_SCALE) return false;
    if (oilScale < SettingsLimits::MIN_PRESSURE_SCALE || oilScale > SettingsLimits::MAX_PRESSURE_SCALE) return false;
    if (hydScale < SettingsLimits::MIN_PRESSURE_SCALE || hydScale > SettingsLimits::MAX_PRESSURE_SCALE) return false;
    if (batteryDivider < SettingsLimits::MIN_BATTERY_DIVIDER || batteryDivider > SettingsLimits::MAX_BATTERY_DIVIDER) return false;
    if (fuelEmpty >= fuelFull || fuelEmpty > SettingsLimits::MAX_FUEL_ADC || fuelFull > SettingsLimits::MAX_FUEL_ADC) return false;
    if (fuelLowThreshold > 100) return false;
    currentSettings.tempSensorScale = tempScale;
    currentSettings.oilPressureScale = oilScale;
    currentSettings.hydPressureScale = hydScale;
    currentSettings.batteryDivider = batteryDivider;
    currentSettings.fuelLevelEmpty = fuelEmpty;
    currentSettings.fuelLevelFull = fuelFull;
    currentSettings.fuelLevelLowThreshold = fuelLowThreshold;
    return true;
}

void SettingsManager::resetCalibrationDefaults() {
    currentSettings.tempSensorScale = TEMP_SENSOR_SCALE;
    currentSettings.oilPressureScale = OIL_PRESSURE_SCALE;
    currentSettings.hydPressureScale = HYD_PRESSURE_SCALE;
    currentSettings.batteryDivider = BATTERY_VOLTAGE_DIVIDER;
    currentSettings.fuelLevelEmpty = (uint16_t)FUEL_LEVEL_EMPTY;
    currentSettings.fuelLevelFull = (uint16_t)FUEL_LEVEL_FULL;
}

bool SettingsManager::updatePinMap(int func, uint8_t gpio, String& err) {
    if (func < 0 || func >= PIN_FUNC_COUNT) { err = "unknown function"; return false; }

    PinType type = PIN_TABLE[func].type;

    // Relays are fixed and cannot be remapped.
    if (type == PT_RELAY) { err = "relay pins are fixed"; return false; }

    // Sensors must be ADC1 pins.
    if (type == PT_ADC) {
        if (!isAdc1Pin(gpio)) { err = "sensor GPIO must be an ADC1 pin (32,33,34,35,36,39)"; return false; }
    } else {
        // Digital in/out must be an exposed, non-strapping, non-relay header pin.
        if (!isHeaderPin(gpio)) { err = "GPIO is not an available header pin"; return false; }
        if (isStrapPin(gpio)) { err = "GPIO is a strapping pin"; return false; }
        if (isRelayGpio(gpio)) { err = "GPIO is reserved for a relay"; return false; }
        // A digital input with pullup cannot use an input-only ADC pin.
        if (type == PT_DIN && (gpio == 34 || gpio == 35 || gpio == 36 || gpio == 39)) {
            err = "input-only pin cannot host a pulled-up digital input"; return false;
        }
    }

    // Reject duplicates across the whole map.
    for (int i = 0; i < PIN_FUNC_COUNT; i++) {
        if (i == func) continue;
        if (currentSettings.pinGpio[i] == gpio) { err = "GPIO already in use by another function"; return false; }
    }
    // Also reject collision with the fixed relay pins.
    if (isRelayGpio(gpio) && type != PT_RELAY) { err = "GPIO is reserved for a relay"; return false; }

    currentSettings.pinGpio[func] = gpio;
    err = "";
    return true;
}

void SettingsManager::addEngineHours(float hours) {
    if (hours <= 0.0f) return;
    currentSettings.engineHours += hours;
}

bool SettingsManager::updateMqtt(uint8_t enabled, const char* host, uint16_t port, const char* topic) {
    currentSettings.mqttEnabled = enabled ? 1 : 0;
    if (host) {
        strncpy(currentSettings.mqttHost, host, sizeof(currentSettings.mqttHost) - 1);
        currentSettings.mqttHost[sizeof(currentSettings.mqttHost) - 1] = '\0';
    }
    if (port > 0) currentSettings.mqttPort = port;
    if (topic) {
        strncpy(currentSettings.mqttTopic, topic, sizeof(currentSettings.mqttTopic) - 1);
        currentSettings.mqttTopic[sizeof(currentSettings.mqttTopic) - 1] = '\0';
    }
    return true;
}

bool SettingsManager::setMqttEnabled(uint8_t v) {
    currentSettings.mqttEnabled = v ? 1 : 0;
    return true;
}

bool SettingsManager::updateTime(uint8_t ntpEnabled, int32_t utcOffset) {
    currentSettings.ntpEnabled = ntpEnabled ? 1 : 0;
    currentSettings.utcOffset = utcOffset;
    return true;
}

bool SettingsManager::validateSettings(const BobcatSettings& settings) {
    if (settings.glowPlugDuration < SettingsLimits::MIN_GLOW_DURATION * 1000UL ||
        settings.glowPlugDuration > SettingsLimits::MAX_GLOW_DURATION * 1000UL) return false;
    if (settings.crankingTimeout < SettingsLimits::MIN_CRANKING_TIMEOUT * 1000UL ||
        settings.crankingTimeout > SettingsLimits::MAX_CRANKING_TIMEOUT * 1000UL) return false;
    if (settings.cooldownDuration < SettingsLimits::MIN_COOLDOWN_DURATION * 1000UL ||
        settings.cooldownDuration > SettingsLimits::MAX_COOLDOWN_DURATION * 1000UL) return false;
    if (settings.glowAssistDuration > SettingsLimits::MAX_GLOW_ASSIST * 1000UL) return false;
    if (settings.maxCoolantTemp < SettingsLimits::MIN_COOLANT_TEMP ||
        settings.maxCoolantTemp > SettingsLimits::MAX_COOLANT_TEMP) return false;
    if (settings.minOilPressure < SettingsLimits::MIN_OIL_PRESSURE_LIMIT ||
        settings.minOilPressure > SettingsLimits::MAX_OIL_PRESSURE_LIMIT) return false;
    if (settings.minHydPressure < SettingsLimits::MIN_HYD_PRESSURE_LIMIT ||
        settings.minHydPressure > SettingsLimits::MAX_HYD_PRESSURE_LIMIT) return false;
    if (settings.minBatteryVoltage < SettingsLimits::MIN_BATTERY_VOLTAGE_LIMIT ||
        settings.minBatteryVoltage > SettingsLimits::MAX_BATTERY_VOLTAGE_LIMIT) return false;
    if (settings.maxBatteryVoltage < SettingsLimits::MIN_BATTERY_VOLTAGE_LIMIT ||
        settings.maxBatteryVoltage > SettingsLimits::MAX_BATTERY_VOLTAGE_LIMIT ||
        settings.maxBatteryVoltage <= settings.minBatteryVoltage) return false;

    // WiFi (empty station SSID allowed; AP SSID required)
    if (strlen(settings.wifiSSID) > SettingsLimits::MAX_SSID_LENGTH) return false;
    if (strlen(settings.apSSID) < SettingsLimits::MIN_AP_SSID_LENGTH ||
        strlen(settings.apSSID) > SettingsLimits::MAX_SSID_LENGTH) return false;

    // Calibration
    if (settings.tempSensorScale < SettingsLimits::MIN_TEMP_SCALE ||
        settings.tempSensorScale > SettingsLimits::MAX_TEMP_SCALE) return false;
    if (settings.oilPressureScale < SettingsLimits::MIN_PRESSURE_SCALE ||
        settings.oilPressureScale > SettingsLimits::MAX_PRESSURE_SCALE) return false;
    if (settings.hydPressureScale < SettingsLimits::MIN_PRESSURE_SCALE ||
        settings.hydPressureScale > SettingsLimits::MAX_PRESSURE_SCALE) return false;
    if (settings.batteryDivider < SettingsLimits::MIN_BATTERY_DIVIDER ||
        settings.batteryDivider > SettingsLimits::MAX_BATTERY_DIVIDER) return false;
    if (settings.fuelLevelEmpty >= settings.fuelLevelFull ||
        settings.fuelLevelEmpty > SettingsLimits::MAX_FUEL_ADC ||
        settings.fuelLevelFull > SettingsLimits::MAX_FUEL_ADC) return false;
    if (settings.fuelLevelLowThreshold > 100) return false;

    // Pin map: each gpio legal for its type, and no duplicates.
    for (int i = 0; i < PIN_FUNC_COUNT; i++) {
        uint8_t g = settings.pinGpio[i];
        PinType t = PIN_TABLE[i].type;
        if (t == PT_RELAY) {
            if (!isRelayGpio(g)) return false;
        } else if (t == PT_ADC) {
            if (!isAdc1Pin(g)) return false;
        } else {
            // Digital I/O: any broken-out header pin, PLUS the onboard LED (GPIO25).
            bool legal = (g == 25) || (isHeaderPin(g) && !isStrapPin(g) && !isRelayGpio(g));
            if (!legal) return false;
            if (t == PT_DIN && (g == 34 || g == 35 || g == 36 || g == 39)) return false;
        }
        for (int j = i + 1; j < PIN_FUNC_COUNT; j++) {
            if (settings.pinGpio[j] == g) return false;
        }
    }
    return true;
}

uint32_t SettingsManager::calculateChecksum(const BobcatSettings& settings) {
    uint32_t checksum = 0;
    const uint8_t* data = reinterpret_cast<const uint8_t*>(&settings);
    size_t size = sizeof(BobcatSettings) - sizeof(uint32_t); // exclude checksum field
    for (size_t i = 0; i < size; i++) {
        checksum ^= data[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    return checksum;
}

bool SettingsManager::validateChecksum(const BobcatSettings& settings) {
    return calculateChecksum(settings) == settings.checksum;
}

bool SettingsManager::performFactoryReset() {
    Serial.println("=== FACTORY RESET ===");
    prefs.clear();
    setDefaultSettings();
    bool result = saveSettings();
    Serial.println(result ? "Factory reset completed successfully" : "Factory reset failed");
    return result;
}

void SettingsManager::logSettingsChange(const char* parameter, const char* oldValue, const char* newValue) {
    Serial.printf("SETTINGS: %s changed from [%s] to [%s]\n", parameter, oldValue, newValue);
}

void SettingsManager::printCurrentSettings() {
    Serial.println("=== CURRENT SETTINGS ===");
    Serial.printf("Engine - Glow: %us, Crank: %us, Cooldown: %us\n",
                  (unsigned)(currentSettings.glowPlugDuration / 1000),
                  (unsigned)(currentSettings.crankingTimeout / 1000),
                  (unsigned)(currentSettings.cooldownDuration / 1000));
    Serial.printf("Alarms - Max Temp: %d C, Min Oil: %d psi, Min Hyd: %d psi, V: %.1f-%.1f\n",
                  currentSettings.maxCoolantTemp, currentSettings.minOilPressure,
                  currentSettings.minHydPressure, currentSettings.minBatteryVoltage,
                  currentSettings.maxBatteryVoltage);
    Serial.printf("WiFi - STA SSID: '%s', AP SSID: '%s'\n",
                  currentSettings.wifiSSID, currentSettings.apSSID);
    Serial.printf("Cal - tempScale: %.4f, oilScale: %.4f, hydScale: %.4f, battDiv: %.5f, fuel: %u-%u\n",
                  currentSettings.tempSensorScale, currentSettings.oilPressureScale,
                  currentSettings.hydPressureScale, currentSettings.batteryDivider,
                  currentSettings.fuelLevelEmpty, currentSettings.fuelLevelFull);
    Serial.print("PinMap:");
    for (int i = 0; i < PIN_FUNC_COUNT; i++) {
        Serial.printf(" %s=%u", PIN_TABLE[i].func, currentSettings.pinGpio[i]);
    }
    Serial.println();
    Serial.printf("Engine hours: %.2f\n", currentSettings.engineHours);
    Serial.printf("Version: %u, Checksum: 0x%08X\n",
                  (unsigned)currentSettings.settingsVersion, currentSettings.checksum);
    Serial.println("========================");
}
