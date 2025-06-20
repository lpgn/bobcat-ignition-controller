/*
 * Configuration Implementation for Bobcat Ignition Controller
 * Defines all constants and global variables for Diesel Engine Control
 */

#include "config.h"

// ============================================================================
// ANALOG INPUT PINS - Engine Sensors (ADC1 channels)
// ============================================================================
const int ENGINE_TEMP_PIN = 36;       // ADC1_CH0 - Coolant Temperature Sensor
const int OIL_PRESSURE_PIN = 39;      // ADC1_CH3 - Oil Pressure Sensor (0-5V)
const int BATTERY_VOLTAGE_PIN = 34;   // ADC1_CH6 - Battery Voltage (via divider)
const int FUEL_LEVEL_PIN = 35;        // ADC1_CH7 - Fuel Level Sensor (0-5V)

// ============================================================================
// DIGITAL INPUT PINS - Status Feedback
// ============================================================================
const int ALTERNATOR_CHARGE_PIN = 27;   // Alternator Charge Indicator
const int ENGINE_RUN_FEEDBACK_PIN = 14;   // Engine Running Feedback

// ============================================================================
// DIESEL ENGINE TIMING CONSTANTS (in milliseconds)
// ============================================================================
const unsigned long GLOW_PLUG_DURATION = 20000;   // 20 seconds glow plug preheat
const unsigned long IGNITION_TIMEOUT = 10000;     // 10 seconds max cranking
const unsigned long COOLDOWN_DURATION = 120000;   // 2 minutes post-shutdown cooldown


// SENSOR CALIBRATION CONSTANTS - Only for sensors we're using

// ============================================================================
// Coolant Temperature Sensor (typical NTC thermistor)
const float TEMP_SENSOR_OFFSET = -40.0;      // Offset for temperature calculation
const float TEMP_SENSOR_SCALE = 0.1953;      // Scale factor (°C per ADC unit)

// Oil Pressure Sensor (0-5V = 0-689 kPa typical, ~0-6.9 bar)
const float OIL_PRESSURE_OFFSET = 0.0;       // Pressure sensor offset
const float OIL_PRESSURE_SCALE = 0.1682;     // 689 kPa / 4095 ADC = 0.1682 kPa/unit

// Battery Voltage Divider (for 12V/24V systems)
const float BATTERY_VOLTAGE_DIVIDER = 0.0111; // (3.3V / 4095) * divider ratio

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

// Global variables
SystemState currentState = IDLE;
unsigned long glowPlugStartTime = 0;
unsigned long ignitionStartTime = 0;
bool startButtonPressed = false;
bool stopButtonPressed = false;

// New button states for power and lights
bool powerOnButtonPressed = false;
bool powerOffButtonPressed = false;

// Non-blocking timing variables
unsigned long shutdownStartTime = 0;
bool shutdownInProgress = false;
