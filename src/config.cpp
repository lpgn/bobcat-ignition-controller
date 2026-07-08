/*
 * Configuration Implementation for Bobcat Ignition Controller
 * Default pin map, legal-GPIO catalogs, and NVS seed constants.
 */

#include "config.h"

// ============================================================================
// AUTHORITATIVE DEFAULT PIN MAP (LilyGO T-Relay 4-Channel board facts)
//   Relays active-high: K1=21 power, K2=19 glow, K3=18 starter, K4=5 lights.
//   Sensors on ADC1: temp=34, oil=35, hydraulic=32, battery=33, fuel=36.
//   Digital: buzzer=26, seat-bar=27, neutral=13(pullup). Status LED=25.
// ============================================================================
const PinInfo PIN_TABLE[PIN_FUNC_COUNT] = {
  { "mainPower",   "Main power",   21, PT_RELAY },
  { "glow",        "Glow plugs",   19, PT_RELAY },
  { "starter",     "Starter",      18, PT_RELAY },
  { "lights",      "Lights",        5, PT_RELAY },
  { "engineTemp",  "Engine temp",  34, PT_ADC   },
  { "oilPressure", "Oil pressure", 35, PT_ADC   },
  { "hydraulic",   "Hydraulic",    32, PT_ADC   },
  { "battery",     "Battery",      33, PT_ADC   },
  { "fuel",        "Fuel level",   36, PT_ADC   },
  { "buzzer",      "Buzzer",       26, PT_DOUT  },
  { "seatBar",     "Seat bar",     27, PT_DIN   },
  { "neutral",     "Neutral",      13, PT_DIN   },
  { "statusLed",   "Status LED",   25, PT_DOUT  },
};

// Exposed header pins (usable for I/O). Strapping pins are listed but flagged.
const uint8_t HEADER_PINS[] = { 2, 4, 12, 13, 14, 15, 22, 23, 26, 27, 32, 33, 34, 35, 36, 39 };
const size_t  HEADER_PINS_COUNT = sizeof(HEADER_PINS) / sizeof(HEADER_PINS[0]);

// ADC1 channels - the only analog-capable pins while WiFi is active.
const uint8_t ADC1_PINS[] = { 32, 33, 34, 35, 36, 39 };
const size_t  ADC1_PINS_COUNT = sizeof(ADC1_PINS) / sizeof(ADC1_PINS[0]);

// Fixed relay pins - cannot be remapped.
const uint8_t RELAY_PINS[] = { 21, 19, 18, 5 };
const size_t  RELAY_PINS_COUNT = sizeof(RELAY_PINS) / sizeof(RELAY_PINS[0]);

// Strapping pins - never use for general I/O.
const uint8_t STRAP_PINS[] = { 2, 4, 12, 15 };
const size_t  STRAP_PINS_COUNT = sizeof(STRAP_PINS) / sizeof(STRAP_PINS[0]);

bool isAdc1Pin(int gpio) {
  for (size_t i = 0; i < ADC1_PINS_COUNT; i++) if (ADC1_PINS[i] == gpio) return true;
  return false;
}

bool isRelayGpio(int gpio) {
  for (size_t i = 0; i < RELAY_PINS_COUNT; i++) if (RELAY_PINS[i] == gpio) return true;
  return false;
}

bool isStrapPin(int gpio) {
  for (size_t i = 0; i < STRAP_PINS_COUNT; i++) if (STRAP_PINS[i] == gpio) return true;
  return false;
}

bool isHeaderPin(int gpio) {
  for (size_t i = 0; i < HEADER_PINS_COUNT; i++) if (HEADER_PINS[i] == gpio) return true;
  return false;
}

const char* pinTypeToString(PinType t) {
  switch (t) {
    case PT_RELAY: return "relay";
    case PT_ADC:   return "adc";
    case PT_DIN:   return "digital-in";
    case PT_DOUT:  return "digital-out";
    default:       return "unknown";
  }
}

// ============================================================================
// FIXED POWER-MANAGEMENT PIN
// ============================================================================
const int WAKE_UP_BUTTON_PIN = 0;   // GPIO0 (BOOT button)

// ============================================================================
// POWER MANAGEMENT CONSTANTS (ms)
// ============================================================================
const unsigned long SLEEP_TIMEOUT = 1800000;      // 30 minutes before auto-sleep
const unsigned long ACTIVITY_TIMEOUT = 300000;    // 5 minutes inactivity before eligibility

// ============================================================================
// DEFAULT DIESEL ENGINE TIMING (ms)
// ============================================================================
const unsigned long GLOW_PLUG_DURATION = 20000;   // 20 s glow preheat
const unsigned long IGNITION_TIMEOUT = 10000;     // 10 s max cranking
const unsigned long COOLDOWN_DURATION = 120000;   // 2 min cooldown
const unsigned long RUNNING_OIL_CONFIRM_MS = 1500; // oil-OK dwell -> RUNNING

// ============================================================================
// DEFAULT SENSOR CALIBRATION (placeholders until characterized)
// ============================================================================
const float TEMP_SENSOR_SCALE = 0.040f;       // °C per ADC unit (inverted NTC)
const float OIL_PRESSURE_SCALE = 0.0195f;     // psi per ADC unit (~80 psi FS)
const float HYD_PRESSURE_SCALE = 0.7326f;     // psi per ADC unit (~3000 psi FS)
const float BATTERY_VOLTAGE_DIVIDER = 0.00526f; // V per ADC unit (56k/10k divider)
const float FUEL_LEVEL_EMPTY = 200.0f;        // ADC value for empty
const float FUEL_LEVEL_FULL = 3800.0f;        // ADC value for full

// ============================================================================
// DEFAULT ENGINE OPERATING PARAMETERS
// ============================================================================
const int MIN_OIL_PRESSURE = 15;      // psi
const int MAX_COOLANT_TEMP = 104;     // °C
const int MIN_HYD_PRESSURE = 800;     // psi
const float MIN_BATTERY_VOLTAGE = 11.5f; // V
const float MAX_BATTERY_VOLTAGE = 14.8f; // V

// ============================================================================
// DISPLAY RANGES
// ============================================================================
const int TEMP_DISPLAY_MIN = 40;      // °C
const int TEMP_DISPLAY_MAX = 120;     // °C
const int OIL_DISPLAY_MAX = 80;       // psi
const int HYD_DISPLAY_MAX = 3000;     // psi
