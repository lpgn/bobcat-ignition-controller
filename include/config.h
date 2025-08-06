/*
 * Configuration Header for Bobcat Ignition Controller
 * Contains all pin definitions, constants, and system configuration
 * Diesel Engine GPIO Mapping for ESP32
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// DIGITAL OUTPUT PINS - Relay Control (Active HIGH) - LILYGO T-Relay 4-Channel
// ============================================================================
const int MAIN_POWER_PIN = 21;        // GPIO21 - Main Power Relay Control (Relay 1)
const int GLOW_PLUGS_PIN = 19;        // GPIO19 - Glow Plug Relay Control (Relay 2)
// const int IGNITION_PIN = 23;       // This is not used in this Bobcat model
const int STARTER_PIN = 18;           // GPIO18 - Starter Solenoid Relay (Relay 3)
const int LIGHTS_PIN = 5;             // GPIO5 - Both Front and Back Lights Relay (Relay 4)

// ============================================================================
// ANALOG INPUT PINS - Engine Sensors (Physical sequence from header left to right)
// ============================================================================
extern const int ENGINE_TEMP_PIN;     // ADC1_CH3 (GPIO39) - Coolant Temperature (5th pin bottom row) - MOVED
extern const int OIL_PRESSURE_PIN;    // ADC1_CH6 (GPIO34) - Oil Pressure Sensor (4th pin top row) - MOVED to GPIO34
extern const int BATTERY_VOLTAGE_PIN; // ADC1_CH0 (GPIO36) - Battery Voltage (4th pin bottom row) - MOVED to GPIO36
extern const int FUEL_LEVEL_PIN;      // ADC1_CH7 (GPIO35) - Fuel Tank Level (5th pin top row) - MOVED

// ============================================================================
// DIGITAL INPUT PINS - Status Feedback (Physical sequence from header)
// ============================================================================
extern const int ALTERNATOR_CHARGE_PIN;    // GPIO22 - Alternator Charge Indicator (1st pin top row)
extern const int ENGINE_RUN_FEEDBACK_PIN;  // GPIO26 - Engine Running Feedback (2nd pin top row)

// ============================================================================
// POWER MANAGEMENT PINS
// ============================================================================
extern const int WAKE_UP_BUTTON_PIN;       // GPIO0 (BOOT button) - Wake up from deep sleep
extern const int SLEEP_ENABLE_PIN;         // GPIO12 - Enable deep sleep mode (optional external control)

// ============================================================================
// POWER MANAGEMENT CONSTANTS
// ============================================================================
extern const unsigned long SLEEP_TIMEOUT;         // Time before auto-sleep (milliseconds)
extern const unsigned long ACTIVITY_TIMEOUT;      // Inactivity timeout for sleep (milliseconds)

// ============================================================================
// DIESEL ENGINE TIMING CONSTANTS
// ============================================================================
extern const unsigned long GLOW_PLUG_DURATION;    // Glow plug preheat time
extern const unsigned long IGNITION_TIMEOUT;      // Max cranking time
extern const unsigned long COOLDOWN_DURATION;     // Post-shutdown cooldown
// SENSOR CALIBRATION CONSTANTS - Only for sensors we're using
// ============================================================================
extern const float TEMP_SENSOR_OFFSET;       // Temperature sensor offset (°C)
extern const float TEMP_SENSOR_SCALE;        // Temperature sensor scale factor
extern const float OIL_PRESSURE_OFFSET;      // Oil pressure sensor offset (kPa)
extern const float OIL_PRESSURE_SCALE;       // Oil pressure sensor scale factor
// Battery Voltage Divider (for 12V/24V systems)
extern const float BATTERY_VOLTAGE_DIVIDER; // TODO: Recalibrate after voltage divider hardware fix
extern const float FUEL_LEVEL_EMPTY;         // ADC value for empty tank
extern const float FUEL_LEVEL_FULL;          // ADC value for full tank

// ============================================================================
// ENGINE OPERATING PARAMETERS
// ============================================================================
extern const int MIN_OIL_PRESSURE;           // Minimum oil pressure (kPa)
extern const int MAX_COOLANT_TEMP;           // Maximum coolant temp (°C)
extern const int MIN_BATTERY_VOLTAGE;        // Minimum battery voltage (12V system)
extern const int MAX_BATTERY_VOLTAGE;        // Maximum battery voltage (12V system)

// System states - Updated to match actual ignition key positions
enum SystemState {
  OFF,                    // Key off - everything off
  ON,                     // Key on - electrical systems active
  GLOW_PLUG,              // Key in glow plug position - heating glow plugs
  START,                  // Key in start position - cranking engine (momentary)
  RUNNING,                // Engine running - key returned to ON position
  LOW_OIL_PRESSURE,
  HIGH_TEMPERATURE,
  ERROR
};

// Global system variables moved to SystemState_t struct
// Access through g_systemState instance in system_state.h

#endif // CONFIG_H
