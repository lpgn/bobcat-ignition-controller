/*
 * Settings Management Implementation for Bobcat Ignition Controller
 * Handles persistent storage and retrieval of user configurable parameters
 */

#include "settings.h"
#include <esp32-hal-log.h>
#include <string.h>

// Global settings manager instance
SettingsManager g_settingsManager;

SettingsManager::SettingsManager() {
    // Initialize with default settings
    setDefaultSettings();
}

bool SettingsManager::begin() {
    // Initialize Preferences library
    if (!prefs.begin("bobcat", false)) {
        Serial.println("ERROR: Failed to initialize settings storage");
        return false;
    }
    
    // Load settings from storage or use defaults
    if (!loadSettings()) {
        Serial.println("WARNING: Failed to load settings, using defaults");
        setDefaultSettings();
        saveSettings(); // Save defaults to storage
    }
    
    Serial.println("Settings Manager initialized successfully");
    printCurrentSettings();
    return true;
}

bool SettingsManager::loadSettings() {
    size_t expectedSize = sizeof(BobcatSettings);
    size_t actualSize = prefs.getBytesLength("settings");
    
    if (actualSize != expectedSize) {
        Serial.printf("Settings size mismatch: expected %d, got %d\n", expectedSize, actualSize);
        return false;
    }
    
    // Load settings from storage
    size_t bytesRead = prefs.getBytes("settings", &currentSettings, expectedSize);
    if (bytesRead != expectedSize) {
        Serial.println("Failed to read settings from storage");
        return false;
    }
    
    // Validate settings version and checksum
    if (currentSettings.settingsVersion != SETTINGS_VERSION) {
        Serial.printf("Settings version mismatch: expected %d, got %d\n", 
                     SETTINGS_VERSION, currentSettings.settingsVersion);
        return false;
    }
    
    if (!validateChecksum(currentSettings)) {
        Serial.println("Settings checksum validation failed - data may be corrupt");
        return false;
    }
    
    // Validate settings values
    if (!validateSettings(currentSettings)) {
        Serial.println("Settings validation failed - using defaults");
        return false;
    }
    
    Serial.println("Settings loaded successfully from storage");
    return true;
}

bool SettingsManager::saveSettings() {
    // Update checksum before saving
    currentSettings.settingsVersion = SETTINGS_VERSION;
    currentSettings.checksum = calculateChecksum(currentSettings);
    
    // Save to preferences
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

void SettingsManager::setDefaultSettings() {
    // Engine Parameters (convert seconds to milliseconds)
    currentSettings.glowPlugDuration = 20000;      // 20 seconds
    currentSettings.crankingTimeout = 10000;       // 10 seconds
    currentSettings.cooldownDuration = 120000;     // 2 minutes
    
    // Alarm Thresholds
    currentSettings.maxCoolantTemp = 104;          // °C
    currentSettings.minOilPressure = 69;           // kPa (~0.7 bar)
    currentSettings.minBatteryVoltage = 11.0f;     // V
    currentSettings.maxBatteryVoltage = 15.0f;     // V
    
    // WiFi Configuration
    strcpy(currentSettings.wifiSSID, "Bobcat-743");
    strcpy(currentSettings.wifiPassword, "bobcat123");
    
    // Sensor Calibration
    currentSettings.tempSensorOffset = -40.0f;     // °C
    currentSettings.pressureScale = 0.1682f;       // kPa per ADC unit
    currentSettings.fuelLevelEmpty = 200;          // ADC value
    currentSettings.fuelLevelFull = 3800;          // ADC value
    
    // System metadata
    currentSettings.settingsVersion = SETTINGS_VERSION;
    currentSettings.checksum = 0; // Will be calculated when saving
}

bool SettingsManager::updateEngineSettings(uint32_t glowDuration, uint32_t crankTimeout, uint32_t cooldown) {
    // Convert from seconds to milliseconds and validate
    uint32_t glowMs = glowDuration * 1000;
    uint32_t crankMs = crankTimeout * 1000;
    uint32_t cooldownMs = cooldown * 1000;
    
    if (glowDuration < SettingsLimits::MIN_GLOW_DURATION || 
        glowDuration > SettingsLimits::MAX_GLOW_DURATION) {
        Serial.printf("Invalid glow duration: %d (must be %d-%d seconds)\n", 
                     glowDuration, SettingsLimits::MIN_GLOW_DURATION, SettingsLimits::MAX_GLOW_DURATION);
        return false;
    }
    
    if (crankTimeout < SettingsLimits::MIN_CRANKING_TIMEOUT || 
        crankTimeout > SettingsLimits::MAX_CRANKING_TIMEOUT) {
        Serial.printf("Invalid cranking timeout: %d (must be %d-%d seconds)\n", 
                     crankTimeout, SettingsLimits::MIN_CRANKING_TIMEOUT, SettingsLimits::MAX_CRANKING_TIMEOUT);
        return false;
    }
    
    if (cooldown < SettingsLimits::MIN_COOLDOWN_DURATION || 
        cooldown > SettingsLimits::MAX_COOLDOWN_DURATION) {
        Serial.printf("Invalid cooldown duration: %d (must be %d-%d seconds)\n", 
                     cooldown, SettingsLimits::MIN_COOLDOWN_DURATION, SettingsLimits::MAX_COOLDOWN_DURATION);
        return false;
    }
    
    // Log changes
    char oldVal[64], newVal[64];
    sprintf(oldVal, "%d/%d/%d", currentSettings.glowPlugDuration/1000, 
            currentSettings.crankingTimeout/1000, currentSettings.cooldownDuration/1000);
    sprintf(newVal, "%d/%d/%d", glowDuration, crankTimeout, cooldown);
    logSettingsChange("Engine Timing", oldVal, newVal);
    
    // Update settings
    currentSettings.glowPlugDuration = glowMs;
    currentSettings.crankingTimeout = crankMs;
    currentSettings.cooldownDuration = cooldownMs;
    
    return true;
}

bool SettingsManager::updateAlarmThresholds(int16_t maxTemp, int16_t minPressure, float minVolt, float maxVolt) {
    // Validate thresholds
    if (maxTemp < SettingsLimits::MIN_COOLANT_TEMP || maxTemp > SettingsLimits::MAX_COOLANT_TEMP) {
        Serial.printf("Invalid max temperature: %d (must be %d-%d°C)\n", 
                     maxTemp, SettingsLimits::MIN_COOLANT_TEMP, SettingsLimits::MAX_COOLANT_TEMP);
        return false;
    }
    
    if (minPressure < SettingsLimits::MIN_OIL_PRESSURE_LIMIT || 
        minPressure > SettingsLimits::MAX_OIL_PRESSURE_LIMIT) {
        Serial.printf("Invalid min oil pressure: %d (must be %d-%d kPa)\n", 
                     minPressure, SettingsLimits::MIN_OIL_PRESSURE_LIMIT, SettingsLimits::MAX_OIL_PRESSURE_LIMIT);
        return false;
    }
    
    if (minVolt < SettingsLimits::MIN_BATTERY_VOLTAGE_LIMIT || 
        minVolt > SettingsLimits::MAX_BATTERY_VOLTAGE_LIMIT) {
        Serial.printf("Invalid min battery voltage: %.1f (must be %.1f-%.1fV)\n", 
                     minVolt, SettingsLimits::MIN_BATTERY_VOLTAGE_LIMIT, SettingsLimits::MAX_BATTERY_VOLTAGE_LIMIT);
        return false;
    }
    
    if (maxVolt < SettingsLimits::MIN_BATTERY_VOLTAGE_LIMIT || 
        maxVolt > SettingsLimits::MAX_BATTERY_VOLTAGE_LIMIT || maxVolt <= minVolt) {
        Serial.printf("Invalid max battery voltage: %.1f (must be %.1f-%.1fV and > min voltage)\n", 
                     maxVolt, SettingsLimits::MIN_BATTERY_VOLTAGE_LIMIT, SettingsLimits::MAX_BATTERY_VOLTAGE_LIMIT);
        return false;
    }
    
    // Log changes
    char oldVal[64], newVal[64];
    sprintf(oldVal, "T:%d P:%d V:%.1f-%.1f", currentSettings.maxCoolantTemp, 
            currentSettings.minOilPressure, currentSettings.minBatteryVoltage, currentSettings.maxBatteryVoltage);
    sprintf(newVal, "T:%d P:%d V:%.1f-%.1f", maxTemp, minPressure, minVolt, maxVolt);
    logSettingsChange("Alarm Thresholds", oldVal, newVal);
    
    // Update settings
    currentSettings.maxCoolantTemp = maxTemp;
    currentSettings.minOilPressure = minPressure;
    currentSettings.minBatteryVoltage = minVolt;
    currentSettings.maxBatteryVoltage = maxVolt;
    
    return true;
}

bool SettingsManager::updateWifiSettings(const char* ssid, const char* password) {
    if (!ssid || strlen(ssid) < SettingsLimits::MIN_SSID_LENGTH || 
        strlen(ssid) > SettingsLimits::MAX_SSID_LENGTH) {
        Serial.printf("Invalid SSID length: %d (must be %d-%d characters)\n", 
                     ssid ? strlen(ssid) : 0, SettingsLimits::MIN_SSID_LENGTH, SettingsLimits::MAX_SSID_LENGTH);
        return false;
    }
    
    if (password && strlen(password) > 0 && 
        (strlen(password) < SettingsLimits::MIN_PASSWORD_LENGTH || 
         strlen(password) > SettingsLimits::MAX_PASSWORD_LENGTH)) {
        Serial.printf("Invalid password length: %d (must be %d-%d characters or empty)\n", 
                     strlen(password), SettingsLimits::MIN_PASSWORD_LENGTH, SettingsLimits::MAX_PASSWORD_LENGTH);
        return false;
    }
    
    // Log changes (don't log passwords)
    logSettingsChange("WiFi SSID", currentSettings.wifiSSID, ssid);
    if (password && strlen(password) > 0) {
        logSettingsChange("WiFi Password", "****", "****");
    }
    
    // Update settings
    strncpy(currentSettings.wifiSSID, ssid, sizeof(currentSettings.wifiSSID) - 1);
    currentSettings.wifiSSID[sizeof(currentSettings.wifiSSID) - 1] = '\0';
    
    if (password) {
        strncpy(currentSettings.wifiPassword, password, sizeof(currentSettings.wifiPassword) - 1);
        currentSettings.wifiPassword[sizeof(currentSettings.wifiPassword) - 1] = '\0';
    }
    
    return true;
}

bool SettingsManager::updateSensorCalibration(float tempOffset, float pressScale, uint16_t fuelEmpty, uint16_t fuelFull) {
    // Validate sensor calibration values
    if (tempOffset < SettingsLimits::MIN_TEMP_OFFSET || tempOffset > SettingsLimits::MAX_TEMP_OFFSET) {
        Serial.printf("Invalid temperature offset: %.1f (must be %.1f-%.1f°C)\n", 
                     tempOffset, SettingsLimits::MIN_TEMP_OFFSET, SettingsLimits::MAX_TEMP_OFFSET);
        return false;
    }
    
    if (pressScale < SettingsLimits::MIN_PRESSURE_SCALE || pressScale > SettingsLimits::MAX_PRESSURE_SCALE) {
        Serial.printf("Invalid pressure scale: %.3f (must be %.3f-%.3f)\n", 
                     pressScale, SettingsLimits::MIN_PRESSURE_SCALE, SettingsLimits::MAX_PRESSURE_SCALE);
        return false;
    }
    
    if (fuelEmpty >= fuelFull || fuelEmpty > SettingsLimits::MAX_FUEL_ADC || fuelFull > SettingsLimits::MAX_FUEL_ADC) {
        Serial.printf("Invalid fuel levels: empty=%d, full=%d (empty must be < full, both 0-%d)\n", 
                     fuelEmpty, fuelFull, SettingsLimits::MAX_FUEL_ADC);
        return false;
    }
    
    // Log changes
    char oldVal[64], newVal[64];
    sprintf(oldVal, "TO:%.1f PS:%.3f FE:%d FF:%d", currentSettings.tempSensorOffset, 
            currentSettings.pressureScale, currentSettings.fuelLevelEmpty, currentSettings.fuelLevelFull);
    sprintf(newVal, "TO:%.1f PS:%.3f FE:%d FF:%d", tempOffset, pressScale, fuelEmpty, fuelFull);
    logSettingsChange("Sensor Calibration", oldVal, newVal);
    
    // Update settings
    currentSettings.tempSensorOffset = tempOffset;
    currentSettings.pressureScale = pressScale;
    currentSettings.fuelLevelEmpty = fuelEmpty;
    currentSettings.fuelLevelFull = fuelFull;
    
    return true;
}

bool SettingsManager::validateSettings(const BobcatSettings& settings) {
    // Validate all settings ranges
    if (settings.glowPlugDuration < SettingsLimits::MIN_GLOW_DURATION * 1000 || 
        settings.glowPlugDuration > SettingsLimits::MAX_GLOW_DURATION * 1000) return false;
    
    if (settings.crankingTimeout < SettingsLimits::MIN_CRANKING_TIMEOUT * 1000 || 
        settings.crankingTimeout > SettingsLimits::MAX_CRANKING_TIMEOUT * 1000) return false;
    
    if (settings.cooldownDuration < SettingsLimits::MIN_COOLDOWN_DURATION * 1000 || 
        settings.cooldownDuration > SettingsLimits::MAX_COOLDOWN_DURATION * 1000) return false;
    
    if (settings.maxCoolantTemp < SettingsLimits::MIN_COOLANT_TEMP || 
        settings.maxCoolantTemp > SettingsLimits::MAX_COOLANT_TEMP) return false;
    
    if (settings.minOilPressure < SettingsLimits::MIN_OIL_PRESSURE_LIMIT || 
        settings.minOilPressure > SettingsLimits::MAX_OIL_PRESSURE_LIMIT) return false;
    
    if (settings.minBatteryVoltage < SettingsLimits::MIN_BATTERY_VOLTAGE_LIMIT || 
        settings.minBatteryVoltage > SettingsLimits::MAX_BATTERY_VOLTAGE_LIMIT) return false;
    
    if (settings.maxBatteryVoltage < SettingsLimits::MIN_BATTERY_VOLTAGE_LIMIT || 
        settings.maxBatteryVoltage > SettingsLimits::MAX_BATTERY_VOLTAGE_LIMIT || 
        settings.maxBatteryVoltage <= settings.minBatteryVoltage) return false;
    
    // Validate WiFi settings
    if (strlen(settings.wifiSSID) < SettingsLimits::MIN_SSID_LENGTH || 
        strlen(settings.wifiSSID) > SettingsLimits::MAX_SSID_LENGTH) return false;
    
    // Validate sensor calibration
    if (settings.tempSensorOffset < SettingsLimits::MIN_TEMP_OFFSET || 
        settings.tempSensorOffset > SettingsLimits::MAX_TEMP_OFFSET) return false;
    
    if (settings.pressureScale < SettingsLimits::MIN_PRESSURE_SCALE || 
        settings.pressureScale > SettingsLimits::MAX_PRESSURE_SCALE) return false;
    
    if (settings.fuelLevelEmpty >= settings.fuelLevelFull || 
        settings.fuelLevelEmpty > SettingsLimits::MAX_FUEL_ADC || 
        settings.fuelLevelFull > SettingsLimits::MAX_FUEL_ADC) return false;
    
    return true;
}

uint32_t SettingsManager::calculateChecksum(const BobcatSettings& settings) {
    uint32_t checksum = 0;
    const uint8_t* data = reinterpret_cast<const uint8_t*>(&settings);
    size_t size = sizeof(BobcatSettings) - sizeof(uint32_t); // Exclude checksum field itself
    
    // Simple XOR checksum
    for (size_t i = 0; i < size; i++) {
        checksum ^= data[i];
        checksum = (checksum << 1) | (checksum >> 31); // Rotate left
    }
    
    return checksum;
}

bool SettingsManager::validateChecksum(const BobcatSettings& settings) {
    uint32_t calculatedChecksum = calculateChecksum(settings);
    return calculatedChecksum == settings.checksum;
}

bool SettingsManager::performFactoryReset() {
    Serial.println("=== FACTORY RESET ===");
    
    // Clear all preferences
    prefs.clear();
    
    // Reset to defaults
    setDefaultSettings();
    
    // Save defaults
    bool result = saveSettings();
    
    Serial.println(result ? "Factory reset completed successfully" : "Factory reset failed");
    return result;
}

void SettingsManager::logSettingsChange(const char* parameter, const char* oldValue, const char* newValue) {
    Serial.printf("SETTINGS: %s changed from [%s] to [%s]\n", parameter, oldValue, newValue);
}

void SettingsManager::printCurrentSettings() {
    Serial.println("=== CURRENT SETTINGS ===");
    Serial.printf("Engine - Glow: %ds, Crank: %ds, Cooldown: %ds\n", 
                 currentSettings.glowPlugDuration/1000, 
                 currentSettings.crankingTimeout/1000, 
                 currentSettings.cooldownDuration/1000);
    Serial.printf("Alarms - Max Temp: %d°C, Min Pressure: %dkPa, Voltage: %.1f-%.1fV\n", 
                 currentSettings.maxCoolantTemp, 
                 currentSettings.minOilPressure, 
                 currentSettings.minBatteryVoltage, 
                 currentSettings.maxBatteryVoltage);
    Serial.printf("WiFi - SSID: %s\n", currentSettings.wifiSSID);
    Serial.printf("Sensors - Temp Offset: %.1f°C, Pressure Scale: %.3f, Fuel: %d-%d\n", 
                 currentSettings.tempSensorOffset, 
                 currentSettings.pressureScale, 
                 currentSettings.fuelLevelEmpty, 
                 currentSettings.fuelLevelFull);
    Serial.printf("Version: %d, Checksum: 0x%08X\n", 
                 currentSettings.settingsVersion, 
                 currentSettings.checksum);
    Serial.println("========================");
}
