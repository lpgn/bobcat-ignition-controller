/*
 * Settings Management Header for Bobcat Ignition Controller
 *
 * SINGLE SOURCE OF TRUTH for all persisted configuration:
 *   engine timing, alarm thresholds, sensor calibration, WiFi, the runtime
 *   pin map, engine hours, and (stored-but-unused) MQTT / time stubs.
 * Persisted in NVS under the "bobcat" namespace.
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

// Settings structure to hold all configurable parameters.
// NOTE: keep `checksum` as the LAST member (checksum excludes only itself).
struct BobcatSettings {
    // Engine Parameters (milliseconds)
    uint32_t glowPlugDuration;
    uint32_t crankingTimeout;
    uint32_t cooldownDuration;

    // Alarm Thresholds
    int16_t maxCoolantTemp;         // °C
    int16_t minOilPressure;         // psi
    int16_t minHydPressure;         // psi (0 -> disabled)
    float   minBatteryVoltage;      // V
    float   maxBatteryVoltage;      // V

    // Station WiFi (empty SSID => station disabled, AP only)
    char wifiSSID[33];
    char wifiPassword[65];

    // Access-Point WiFi
    char apSSID[33];
    char apPassword[65];

    // Sensor Calibration (unified - sensor reads use exactly these)
    float    tempSensorScale;       // °C per ADC unit (inverted NTC)
    float    oilPressureScale;      // psi per ADC unit
    float    hydPressureScale;      // psi per ADC unit
    float    batteryDivider;        // V per ADC unit
    uint16_t fuelLevelEmpty;        // ADC value empty
    uint16_t fuelLevelFull;         // ADC value full
    uint8_t  fuelLevelLowThreshold; // %

    // Runtime pin map (GPIO per PinFunc)
    uint8_t pinGpio[PIN_FUNC_COUNT];

    // Engine hours (accumulated while RUNNING)
    float engineHours;

    // MQTT stub (stored, unused until the MQTT feature lands)
    uint8_t  mqttEnabled;
    char     mqttHost[64];
    uint16_t mqttPort;
    char     mqttTopic[64];

    // Time / NTP stub
    uint8_t ntpEnabled;
    int32_t utcOffset;              // seconds

    // System metadata
    uint32_t settingsVersion;
    uint32_t checksum;             // MUST be last
};

class SettingsManager {
public:
    SettingsManager();

    bool begin();
    bool loadSettings();
    bool saveSettings();
    bool resetToDefaults();

    const BobcatSettings& getSettings() const { return currentSettings; }

    // Scalar getters
    uint32_t getGlowPlugDuration() const { return currentSettings.glowPlugDuration; }
    uint32_t getCrankingTimeout() const { return currentSettings.crankingTimeout; }
    uint32_t getCooldownDuration() const { return currentSettings.cooldownDuration; }
    int16_t getMaxCoolantTemp() const { return currentSettings.maxCoolantTemp; }
    int16_t getMinOilPressure() const { return currentSettings.minOilPressure; }
    int16_t getMinHydPressure() const { return currentSettings.minHydPressure; }
    float getMinBatteryVoltage() const { return currentSettings.minBatteryVoltage; }
    float getMaxBatteryVoltage() const { return currentSettings.maxBatteryVoltage; }
    const char* getWifiSSID() const { return currentSettings.wifiSSID; }
    const char* getWifiPassword() const { return currentSettings.wifiPassword; }
    const char* getApSSID() const { return currentSettings.apSSID; }
    const char* getApPassword() const { return currentSettings.apPassword; }
    float getTempSensorScale() const { return currentSettings.tempSensorScale; }
    float getOilPressureScale() const { return currentSettings.oilPressureScale; }
    float getHydPressureScale() const { return currentSettings.hydPressureScale; }
    float getBatteryDivider() const { return currentSettings.batteryDivider; }
    uint16_t getFuelLevelEmpty() const { return currentSettings.fuelLevelEmpty; }
    uint16_t getFuelLevelFull() const { return currentSettings.fuelLevelFull; }
    uint8_t getFuelLevelLowThreshold() const { return currentSettings.fuelLevelLowThreshold; }
    uint8_t getPinGpio(int func) const;
    float getEngineHours() const { return currentSettings.engineHours; }

    // Mutators (RAM only - caller flags configDirty; loop() persists)
    bool updateEngineSettings(uint32_t glowDurationMs, uint32_t crankTimeoutMs, uint32_t cooldownMs);
    bool updateAlarmThresholds(int16_t maxTemp, int16_t minOilPsi, float minVolt, float maxVolt);
    bool updateHydraulicThreshold(int16_t minHydPsi);
    bool updateWifiSettings(const char* ssid, const char* password);
    bool updateApSettings(const char* ssid, const char* password);
    bool updateCalibration(float tempScale, float oilScale, float hydScale,
                           float batteryDivider, uint16_t fuelEmpty, uint16_t fuelFull,
                           uint8_t fuelLowThreshold);
    bool setTempScale(float v);
    bool setOilScale(float v);
    bool setHydScale(float v);
    bool setBatteryDivider(float v);
    bool setFuelEmpty(uint16_t v);
    bool setFuelFull(uint16_t v);
    bool setFuelLowThreshold(uint8_t v);
    bool updatePinMap(int func, uint8_t gpio, String& err);  // validates
    void addEngineHours(float hours);
    void resetCalibrationDefaults();

    // MQTT / time stubs
    bool updateMqtt(uint8_t enabled, const char* host, uint16_t port, const char* topic);
    bool setMqttEnabled(uint8_t v);
    bool updateTime(uint8_t ntpEnabled, int32_t utcOffset);

    bool validateSettings(const BobcatSettings& settings);
    bool performFactoryReset();
    void printCurrentSettings();

private:
    Preferences prefs;
    BobcatSettings currentSettings;

    void setDefaultSettings();
    void setDefaultPinMap();
    uint32_t calculateChecksum(const BobcatSettings& settings);
    bool validateChecksum(const BobcatSettings& settings);
    void logSettingsChange(const char* parameter, const char* oldValue, const char* newValue);
};

extern SettingsManager g_settingsManager;

// Validation limits
namespace SettingsLimits {
    constexpr uint8_t MIN_GLOW_DURATION = 5;
    constexpr uint8_t MAX_GLOW_DURATION = 60;
    constexpr uint8_t MIN_CRANKING_TIMEOUT = 5;
    constexpr uint8_t MAX_CRANKING_TIMEOUT = 30;
    constexpr uint16_t MIN_COOLDOWN_DURATION = 60;
    constexpr uint16_t MAX_COOLDOWN_DURATION = 300;

    constexpr int16_t MIN_COOLANT_TEMP = 80;
    constexpr int16_t MAX_COOLANT_TEMP = 120;
    constexpr int16_t MIN_OIL_PRESSURE_LIMIT = 5;
    constexpr int16_t MAX_OIL_PRESSURE_LIMIT = 150;
    constexpr int16_t MIN_HYD_PRESSURE_LIMIT = 0;
    constexpr int16_t MAX_HYD_PRESSURE_LIMIT = 3000;

    constexpr float MIN_BATTERY_VOLTAGE_LIMIT = 10.0f;
    constexpr float MAX_BATTERY_VOLTAGE_LIMIT = 16.0f;

    constexpr uint8_t MAX_SSID_LENGTH = 32;    // 0 allowed => disabled
    constexpr uint8_t MIN_AP_SSID_LENGTH = 1;
    constexpr uint8_t MIN_PASSWORD_LENGTH = 8;
    constexpr uint8_t MAX_PASSWORD_LENGTH = 64;

    constexpr float MIN_TEMP_SCALE = 0.001f;
    constexpr float MAX_TEMP_SCALE = 10.0f;
    constexpr float MIN_PRESSURE_SCALE = 0.001f;
    constexpr float MAX_PRESSURE_SCALE = 10.0f;
    constexpr float MIN_BATTERY_DIVIDER = 0.0001f;
    constexpr float MAX_BATTERY_DIVIDER = 1.0f;
    constexpr uint16_t MAX_FUEL_ADC = 4095;
}

// Bumped to 2: struct layout changed (pin map, unified calibration, stubs).
constexpr uint32_t SETTINGS_VERSION = 2;

#endif // SETTINGS_H
