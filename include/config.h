/*
 * Configuration Header for Bobcat Ignition Controller
 * Board: LilyGO T-Relay 4-Channel (ESP32)
 *
 * NOTE: Physical GPIO assignments are NO LONGER hard-coded here. They live in a
 * runtime pin map stored in NVS (see SettingsManager). This header only provides
 * the pin-function catalog, the board's authoritative default map, the set of
 * legal GPIOs, and the compile-time DEFAULT values used to seed NVS on first boot.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// PIN FUNCTION CATALOG (runtime pin map is indexed by this enum)
// ============================================================================
enum PinFunc {
  PIN_MAIN_POWER = 0,   // Relay K1 (active-high) - main power
  PIN_GLOW,             // Relay K2 - glow plugs
  PIN_STARTER,          // Relay K3 - starter solenoid
  PIN_LIGHTS,           // Relay K4 - work lights
  PIN_ENGINE_TEMP,      // ADC1 - coolant temperature
  PIN_OIL_PRESSURE,     // ADC1 - oil pressure
  PIN_HYDRAULIC,        // ADC1 - hydraulic pressure
  PIN_BATTERY,          // ADC1 - battery sense
  PIN_FUEL,             // ADC1 - fuel level
  PIN_BUZZER,           // Digital out - alarm buzzer
  PIN_SEAT_BAR,         // Digital in (pullup) - seat-bar interlock
  PIN_NEUTRAL,          // Digital in (pullup) - neutral interlock
  PIN_STATUS_LED,       // Digital out - onboard status LED
  PIN_FUNC_COUNT
};

// Electrical class of a pin function - drives pinMode + validation.
enum PinType {
  PT_RELAY = 0,   // fixed active-high relay output
  PT_ADC,         // analog input (ADC1 only)
  PT_DIN,         // digital input with pullup
  PT_DOUT         // digital output (buzzer / LED)
};

struct PinInfo {
  const char* func;      // machine key, e.g. "engineTemp"
  const char* label;     // human label, e.g. "Engine temp"
  uint8_t     defaultGpio;
  PinType     type;
};

// Authoritative board default map (see board pin facts).
extern const PinInfo PIN_TABLE[PIN_FUNC_COUNT];

// Legal-GPIO catalogs used by /api/pins and by write validation.
extern const uint8_t HEADER_PINS[];   extern const size_t HEADER_PINS_COUNT;
extern const uint8_t ADC1_PINS[];     extern const size_t ADC1_PINS_COUNT;
extern const uint8_t RELAY_PINS[];    extern const size_t RELAY_PINS_COUNT;
extern const uint8_t STRAP_PINS[];    extern const size_t STRAP_PINS_COUNT;

// GPIO classification helpers (defined in config.cpp).
bool isAdc1Pin(int gpio);       // one of ADC1 {32,33,34,35,36,39}
bool isRelayGpio(int gpio);     // one of the fixed relay pins {21,19,18,5}
bool isStrapPin(int gpio);      // strapping pin {2,4,12,15}
bool isHeaderPin(int gpio);     // exposed header pin

const char* pinTypeToString(PinType t);

// ============================================================================
// FIXED POWER-MANAGEMENT PIN (BOOT button, not part of the runtime map)
// ============================================================================
extern const int WAKE_UP_BUTTON_PIN;   // GPIO0 - wake from deep sleep

// ============================================================================
// POWER MANAGEMENT CONSTANTS (ms)
// ============================================================================
extern const unsigned long SLEEP_TIMEOUT;
extern const unsigned long ACTIVITY_TIMEOUT;

// ============================================================================
// DEFAULT DIESEL ENGINE TIMING (ms) - seed values for NVS only.
// The state machine reads the live values from SettingsManager.
// ============================================================================
extern const unsigned long GLOW_PLUG_DURATION;
extern const unsigned long IGNITION_TIMEOUT;
extern const unsigned long COOLDOWN_DURATION;

// Time oil pressure must stay OK while cranking before declaring RUNNING.
extern const unsigned long RUNNING_OIL_CONFIRM_MS;

// ============================================================================
// DEFAULT SENSOR CALIBRATION - seed values for NVS only.
// ============================================================================
extern const float TEMP_SENSOR_SCALE;        // °C per ADC unit (inverted NTC)
extern const float OIL_PRESSURE_SCALE;        // psi per ADC unit
extern const float HYD_PRESSURE_SCALE;        // psi per ADC unit
extern const float BATTERY_VOLTAGE_DIVIDER;   // V per ADC unit
extern const float FUEL_LEVEL_EMPTY;          // ADC value for empty tank
extern const float FUEL_LEVEL_FULL;           // ADC value for full tank

// ============================================================================
// DEFAULT ENGINE OPERATING PARAMETERS (thresholds) - seed values for NVS only.
// ============================================================================
extern const int MIN_OIL_PRESSURE;      // psi
extern const int MAX_COOLANT_TEMP;      // °C
extern const int MIN_HYD_PRESSURE;      // psi
extern const float MIN_BATTERY_VOLTAGE; // V
extern const float MAX_BATTERY_VOLTAGE; // V

// ============================================================================
// DISPLAY RANGES for /api/status gauges (not thresholds).
// ============================================================================
extern const int TEMP_DISPLAY_MIN;      // °C
extern const int TEMP_DISPLAY_MAX;      // °C
extern const int OIL_DISPLAY_MAX;       // psi
extern const int HYD_DISPLAY_MAX;       // psi

// ============================================================================
// SYSTEM STATES - match actual ignition key positions
// ============================================================================
enum SystemState {
  OFF,                    // Key off - everything off
  ON,                     // Key on - electrical systems active
  GLOW_PLUG,              // Glow plug preheat
  START,                  // Cranking (momentary)
  RUNNING,                // Engine running
  LOW_OIL_PRESSURE,       // Running + low-oil alert
  HIGH_TEMPERATURE,       // Running + over-temp alert
  ERROR
};

#endif // CONFIG_H
