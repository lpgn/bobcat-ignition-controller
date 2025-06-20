/*
 * Configuration Header for Bobcat Ignition Controller
 * Contains all pin definitions, constants, and system configuration
 * Diesel Engine GPIO Mapping for ESP32
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// DIGITAL OUTPUT PINS - Relay Control (Active HIGH)
// ============================================================================
const int MAIN_POWER_PIN = 5;         // GPIO5 - Main Power Relay Control
const int GLOW_PLUGS_PIN = 21;        // GPIO21 - Glow Plug Relay Control
// const int IGNITION_PIN = 23;       // This is not used in this Bobcat model
const int STARTER_PIN = 22;           // GPIO22 - Starter Solenoid Relay
const int LIGHTS_PIN = 18;            // GPIO18 - Both Front and Back Lights Relay

// ============================================================================
// ANALOG INPUT PINS - Engine Sensors (ADC1 channels for WiFi compatibility)
// ============================================================================
extern const int ENGINE_TEMP_PIN;     // ADC1_CH0 (GPIO36) - Coolant Temperature
extern const int OIL_PRESSURE_PIN;    // ADC1_CH3 (GPIO39) - Oil Pressure Sensor
extern const int BATTERY_VOLTAGE_PIN; // ADC1_CH6 (GPIO34) - Battery Voltage (12V/24V)
extern const int FUEL_LEVEL_PIN;      // ADC1_CH7 (GPIO35) - Fuel Tank Level

// ============================================================================
// DIGITAL INPUT PINS - Status Feedback
// ============================================================================
extern const int ALTERNATOR_CHARGE_PIN;    // GPIO27 - Alternator Charge Indicator
extern const int ENGINE_RUN_FEEDBACK_PIN;  // GPIO14 - Engine Running Feedback

// ============================================================================// DIESEL ENGINE TIMING CONSTANTS
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
extern const float BATTERY_VOLTAGE_DIVIDER;  // Voltage divider ratio
extern const float FUEL_LEVEL_EMPTY;         // ADC value for empty tank
extern const float FUEL_LEVEL_FULL;          // ADC value for full tank

// ============================================================================
// ENGINE OPERATING PARAMETERS
// ============================================================================
extern const int MIN_OIL_PRESSURE;           // Minimum oil pressure (kPa)
extern const int MAX_COOLANT_TEMP;           // Maximum coolant temp (°C)
extern const int MIN_BATTERY_VOLTAGE;        // Minimum battery voltage (12V system)
extern const int MAX_BATTERY_VOLTAGE;        // Maximum battery voltage (12V system)

// System states
enum SystemState {
  IDLE,
  POWER_ON,
  GLOW_PLUG_HEATING,
  READY_TO_START,
  STARTING,
  RUNNING,
  LOW_OIL_PRESSURE,
  HIGH_TEMPERATURE,
  ERROR
};

// Global system variables
extern SystemState currentState;
extern unsigned long glowPlugStartTime;
extern unsigned long ignitionStartTime;
extern bool startButtonPressed;
extern bool stopButtonPressed;

// New button states for power and lights
extern bool powerOnButtonPressed;
extern bool powerOffButtonPressed;

// Non-blocking timing variables
extern unsigned long shutdownStartTime;
extern bool shutdownInProgress;

#endif // CONFIG_H
