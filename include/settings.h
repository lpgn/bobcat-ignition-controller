/*
 * Settings Management Header for Bobcat Ignition Controller
 * Handles persistent storage and retrieval of user configurable parameters
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

// Settings structure to hold all configurable parameters
struct BobcatSettings {
    // Engine Parameters (in milliseconds for internal use)
    uint32_t glowPlugDuration;      // Glow plug heating time (default: 20000ms)
    uint32_t crankingTimeout;       // Maximum cranking time (default: 10000ms)
    uint32_t cooldownDuration;      // Post-shutdown cooldown (default: 120000ms)
    
    // Alarm Thresholds
    int16_t maxCoolantTemp;         // Maximum coolant temperature °C (default: 104)
    int16_t minOilPressure;         // Minimum oil pressure kPa (default: 69)
    float minBatteryVoltage;        // Minimum battery voltage V (default: 11.0)
    float maxBatteryVoltage;        // Maximum battery voltage V (default: 15.0)
    
    // WiFi Configuration
    char wifiSSID[33];              // WiFi SSID (max 32 chars + null)
    char wifiPassword[65];          // WiFi password (max 64 chars + null)
    
    // Sensor Calibration
    float tempSensorOffset;         // Temperature sensor offset °C (default: -40.0)
    float pressureScale;            // Oil pressure scale kPa/unit (default: 0.1682)
    uint16_t fuelLevelEmpty;        // Fuel empty ADC value (default: 200)
    uint16_t fuelLevelFull;         // Fuel full ADC value (default: 3800)
    
    // System metadata
    uint32_t settingsVersion;       // Settings format version for future upgrades
    uint32_t checksum;              // Data integrity checksum
};

// Settings management class
class SettingsManager {
public:
    SettingsManager();
    
    // Initialization and management
    bool begin();
    bool loadSettings();
    bool saveSettings();
    bool resetToDefaults();
    
    // Getters for current settings
    const BobcatSettings& getSettings() const { return currentSettings; }
    
    // Individual parameter getters (for easy access)
    uint32_t getGlowPlugDuration() const { return currentSettings.glowPlugDuration; }
    uint32_t getCrankingTimeout() const { return currentSettings.crankingTimeout; }
    uint32_t getCooldownDuration() const { return currentSettings.cooldownDuration; }
    int16_t getMaxCoolantTemp() const { return currentSettings.maxCoolantTemp; }
    int16_t getMinOilPressure() const { return currentSettings.minOilPressure; }
    float getMinBatteryVoltage() const { return currentSettings.minBatteryVoltage; }
    float getMaxBatteryVoltage() const { return currentSettings.maxBatteryVoltage; }
    const char* getWifiSSID() const { return currentSettings.wifiSSID; }
    const char* getWifiPassword() const { return currentSettings.wifiPassword; }
    float getTempSensorOffset() const { return currentSettings.tempSensorOffset; }
    float getPressureScale() const { return currentSettings.pressureScale; }
    uint16_t getFuelLevelEmpty() const { return currentSettings.fuelLevelEmpty; }
    uint16_t getFuelLevelFull() const { return currentSettings.fuelLevelFull; }
    
    // Setters for updating settings
    bool updateEngineSettings(uint32_t glowDuration, uint32_t crankTimeout, uint32_t cooldown);
    bool updateAlarmThresholds(int16_t maxTemp, int16_t minPressure, float minVolt, float maxVolt);
    bool updateWifiSettings(const char* ssid, const char* password);
    bool updateSensorCalibration(float tempOffset, float pressScale, uint16_t fuelEmpty, uint16_t fuelFull);
    
    // Validation functions
    bool validateSettings(const BobcatSettings& settings);
    bool isSettingsCorrupt();
    
    // Factory reset and recovery
    bool performFactoryReset();
    void printCurrentSettings();
    
private:
    Preferences prefs;
    BobcatSettings currentSettings;
    
    // Internal methods
    void setDefaultSettings();
    uint32_t calculateChecksum(const BobcatSettings& settings);
    bool validateChecksum(const BobcatSettings& settings);
    void logSettingsChange(const char* parameter, const char* oldValue, const char* newValue);
};

// Global settings manager instance
extern SettingsManager g_settingsManager;

// Constants for settings validation
namespace SettingsLimits {
    // Engine timing limits (in seconds for user interface)
    constexpr uint8_t MIN_GLOW_DURATION = 5;
    constexpr uint8_t MAX_GLOW_DURATION = 60;
    constexpr uint8_t MIN_CRANKING_TIMEOUT = 5;
    constexpr uint8_t MAX_CRANKING_TIMEOUT = 30;
    constexpr uint16_t MIN_COOLDOWN_DURATION = 60;
    constexpr uint16_t MAX_COOLDOWN_DURATION = 300;
    
    // Temperature and pressure limits
    constexpr int16_t MIN_COOLANT_TEMP = 80;
    constexpr int16_t MAX_COOLANT_TEMP = 120;
    constexpr int16_t MIN_OIL_PRESSURE_LIMIT = 30;
    constexpr int16_t MAX_OIL_PRESSURE_LIMIT = 150;
    
    // Voltage limits
    constexpr float MIN_BATTERY_VOLTAGE_LIMIT = 10.0f;
    constexpr float MAX_BATTERY_VOLTAGE_LIMIT = 16.0f;
    
    // WiFi limits
    constexpr uint8_t MIN_SSID_LENGTH = 1;
    constexpr uint8_t MAX_SSID_LENGTH = 32;
    constexpr uint8_t MIN_PASSWORD_LENGTH = 8;
    constexpr uint8_t MAX_PASSWORD_LENGTH = 64;
    
    // Sensor calibration limits
    constexpr float MIN_TEMP_OFFSET = -50.0f;
    constexpr float MAX_TEMP_OFFSET = 50.0f;
    constexpr float MIN_PRESSURE_SCALE = 0.01f;
    constexpr float MAX_PRESSURE_SCALE = 1.0f;
    constexpr uint16_t MIN_FUEL_ADC = 0;
    constexpr uint16_t MAX_FUEL_ADC = 4095;
}

// Settings version for future compatibility
constexpr uint32_t SETTINGS_VERSION = 1;

#endif // SETTINGS_H
