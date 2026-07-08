/*
 * Hardware Control Header for Bobcat Ignition Controller
 * Relay/sensor/interlock access through the runtime pin map.
 */

#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include "config.h"
#include "system_state.h"

// ============================================================================
// RUNTIME CALIBRATION (mirrors SettingsManager - the single source of truth)
// ============================================================================
extern float runtime_battery_divider;
extern float runtime_temp_scale;
extern float runtime_pressure_scale;
extern float runtime_hyd_pressure_scale;
extern int   runtime_fuel_empty;
extern int   runtime_fuel_full;

// ============================================================================
// HARDWARE INITIALIZATION
// ============================================================================
void initializePins();            // applies pin map + calibration from settings
void loadCalibrationConstants();  // refresh runtime_* from settings
void applyPinMap();               // refresh runtime pin table from settings

// Runtime pin-map accessors
int  pinFor(PinFunc f);           // current GPIO for a function
bool outputOn(PinFunc f);         // digitalRead of an output pin (status only)

// ============================================================================
// RELAY / OUTPUT CONTROL
// ============================================================================
void controlMainPower(bool enable);
void controlGlowPlugs(bool enable);
void controlStarter(bool enable);
void controlLights(bool enable);
void controlBuzzer(bool enable);
void controlStatusLed(bool enable);

// ============================================================================
// VIRTUAL BUTTONS (web interface)
// ============================================================================
void virtualPowerOnButton();
void virtualPowerOffButton();
void virtualStartButton();
void virtualLightsButton();

// ============================================================================
// SENSOR READS (one formula per sensor, from unified calibration)
// ============================================================================
float readEngineTemp();        // °C
float readOilPressure();       // psi (real analog read)
float readHydraulicPressure(); // psi (real analog read)
float readBatteryVoltage();    // V
float readFuelLevel();         // % (clamped 0-100)

// ============================================================================
// SAFETY INTERLOCKS (MANDATORY before every starter engagement)
// ============================================================================
bool readSeatBarSafety();          // true = operator seated
bool readNeutralSafety();          // true = transmission in neutral
bool safetyInterlocksPassed();     // seat + neutral
bool batteryOkToStart();           // battery >= settings.minBatteryVoltage

// ============================================================================
// ENGINE STATE HELPERS
// ============================================================================
bool isEngineRunning();            // reflects the state machine (no dev override)
void performSafetyShutdown();

// ============================================================================
// POWER MANAGEMENT
// ============================================================================
void initializeSleepMode();
void enterDeepSleep();
void prepareForSleep();
bool checkSleepConditions();
bool checkSleepConditions(bool manualSleep);
void handleWakeUp();
void updateActivityTimer();

#endif // HARDWARE_H
