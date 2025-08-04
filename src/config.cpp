/*
 * C// ============================================================================
// ANALOG INPUT PINS - Engine Sensors (Physical sequence from header)
// ============================================================================
const int ENGINE_TEMP_PIN = 39;       // ADC1_CH3 - Coolant Temperature Sensor (4th pin top row)
const int OIL_PRESSURE_PIN = 35;      // ADC1_CH7 - Oil Pressure Sensor (5th pin top row)
const int BATTERY_VOLTAGE_PIN = 36;   // ADC1_CH0 - Battery Voltage (4th pin bottom row)
const int FUEL_LEVEL_PIN = 34;        // ADC1_CH6 - Fuel Level Sensor (5th pin bottom row)ation Implementation for Bobcat Ignition Controller
 * Defines all constants and global variables for Diesel Engine Control
 */

#include "config.h"

// ============================================================================
// ANALOG INPUT PINS - Engine Sensors (ADC1 channels) - Sequential assignment
// ============================================================================
const int ENGINE_TEMP_PIN = 34;       // ADC1_CH6 - Coolant Temperature Sensor
const int OIL_PRESSURE_PIN = 35;      // ADC1_CH7 - Oil Pressure Sensor (0-5V)
const int BATTERY_VOLTAGE_PIN = 36;   // ADC1_CH0 - Battery Voltage (via divider)
const int FUEL_LEVEL_PIN = 39;        // ADC1_CH3 - Fuel Level Sensor (0-5V)

// ============================================================================
// DIGITAL INPUT PINS - Status Feedback (Physical sequence from header)
// ============================================================================
const int ALTERNATOR_CHARGE_PIN = 22;     // GPIO22 - Alternator Charge Indicator (1st pin top row)
const int ENGINE_RUN_FEEDBACK_PIN = 26;   // GPIO26 - Engine Running Feedback (2nd pin top row)

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
SystemState currentState = OFF;
unsigned long glowPlugStartTime = 0;
unsigned long ignitionStartTime = 0;

// Key position variables - simulate actual ignition key
int keyPosition = 0;                    // 0=OFF, 1=ON, 2=GLOW_PLUG, 3=START
bool keyStartHeld = false;              // True while START position is held
unsigned long startHoldTime = 0;       // When START position was first engaged

// Button states for emergency stop and lights
bool emergencyStopPressed = false;
bool lightsTogglePressed = false;
bool workLightsOn = false;              // Current state of work lights

// Non-blocking timing variables
unsigned long shutdownStartTime = 0;
bool shutdownInProgress = false;
