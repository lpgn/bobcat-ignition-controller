/*
 * Configuration Implementation for Bobcat Ignition Controller
 * Defines all constants and global variables for Diesel Engine Control
 */

#include "config.h"

// ============================================================================
// ANALOG INPUT PINS - Engine Sensors (ADC1 channels) - Sequential assignment
// ============================================================================
const int ENGINE_TEMP_PIN = 39;       // ADC1_CH3 - Coolant Temperature Sensor - MOVED
const int OIL_PRESSURE_PIN = 34;      // ADC1_CH6 - Oil Pressure Sensor (0-5V) - MOVED to GPIO34
const int BATTERY_VOLTAGE_PIN = 36;   // ADC1_CH0 - Battery Voltage (via divider) - MOVED to GPIO36
const int FUEL_LEVEL_PIN = 35;        // ADC1_CH7 - Fuel Level Sensor (0-5V) - MOVED

// ============================================================================
// DIGITAL INPUT PINS - Status Feedback (Physical sequence from header)
// ============================================================================
const int ALTERNATOR_CHARGE_PIN = 22;     // GPIO22 - Alternator Charge Indicator (1st pin top row)
const int ENGINE_RUN_FEEDBACK_PIN = 26;   // GPIO26 - Engine Running Feedback (2nd pin top row)

// ============================================================================
// POWER MANAGEMENT PINS
// ============================================================================
const int WAKE_UP_BUTTON_PIN = 0;         // GPIO0 (BOOT button) - Wake up from deep sleep
const int SLEEP_ENABLE_PIN = 12;          // GPIO12 - Enable deep sleep mode (optional external control)

// ============================================================================
// DIESEL ENGINE TIMING CONSTANTS (in milliseconds)
// ============================================================================
const unsigned long GLOW_PLUG_DURATION = 20000;   // 20 seconds glow plug preheat
const unsigned long IGNITION_TIMEOUT = 10000;     // 10 seconds max cranking
const unsigned long COOLDOWN_DURATION = 120000;   // 2 minutes post-shutdown cooldown

// ============================================================================
// POWER MANAGEMENT CONSTANTS (in milliseconds)
// ============================================================================
const unsigned long SLEEP_TIMEOUT = 1800000;      // 30 minutes before auto-sleep
const unsigned long ACTIVITY_TIMEOUT = 300000;    // 5 minutes of inactivity before sleep eligibility


// SENSOR CALIBRATION CONSTANTS - Only for sensors we're using

// ============================================================================
// Coolant Temperature Sensor (typical NTC thermistor)
const float TEMP_SENSOR_OFFSET = -40.0;      // Offset for temperature calculation (legacy, not used in new formula)
const float TEMP_SENSOR_SCALE = 0.040;       // Scale factor for inverted NTC formula (°C per ADC unit)

// Oil Pressure Sensor (0-5V = 0-689 kPa typical, ~0-6.9 bar)
const float OIL_PRESSURE_OFFSET = 0.0;       // Pressure sensor offset
const float OIL_PRESSURE_SCALE = 0.1682;     // 689 kPa / 4095 ADC = 0.1682 kPa/unit

// Battery Voltage Divider (for 12V/24V systems)
// Calibrated with 56kΩ + 10kΩ voltage divider
// Measured: 13.06V battery → 2.0V ADC input → ~2482 ADC reading
// Voltage divider constant: 13.06V ÷ 2482 = 0.00526
const float BATTERY_VOLTAGE_DIVIDER = 0.00526; // Calibrated for 56kΩ/10kΩ divider

// Fuel Level Sensor Calibration
const float FUEL_LEVEL_EMPTY = 200.0;        // ADC reading for empty tank
const float FUEL_LEVEL_FULL = 3800.0;        // ADC reading for full tank

// ============================================================================
// Global variables
// ============================================================================
const int MIN_OIL_PRESSURE = 69;             // Minimum oil pressure (kPa, ~0.7 bar)
const int MAX_COOLANT_TEMP = 104;            // Maximum coolant temp (°C)
const int MIN_BATTERY_VOLTAGE = 11;          // Minimum battery voltage (12V system)
const int MAX_BATTERY_VOLTAGE = 15;          // Maximum battery voltage (12V system)

// Global variables moved to SystemState_t struct in system_state.cpp
// All state management now centralized through g_systemState
