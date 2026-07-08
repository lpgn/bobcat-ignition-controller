/*
 * Hardware Control Implementation for Bobcat Ignition Controller
 */

#include "hardware.h"
#include "config.h"
#include "system_state.h"
#include "settings.h"

// Runtime calibration (mirrors SettingsManager)
float runtime_battery_divider = BATTERY_VOLTAGE_DIVIDER;
float runtime_temp_scale = TEMP_SENSOR_SCALE;
float runtime_pressure_scale = OIL_PRESSURE_SCALE;
float runtime_hyd_pressure_scale = HYD_PRESSURE_SCALE;
int   runtime_fuel_empty = (int)FUEL_LEVEL_EMPTY;
int   runtime_fuel_full = (int)FUEL_LEVEL_FULL;

// Runtime pin table (applied from settings at boot)
static int g_runtimePins[PIN_FUNC_COUNT];
static bool g_pinsApplied = false;

// Temperature sensor moving-average filter
#define TEMP_FILTER_SIZE 10
static float tempReadings[TEMP_FILTER_SIZE];
static int tempIndex = 0;
static bool tempFilterInitialized = false;

int pinFor(PinFunc f) {
  if (f < 0 || f >= PIN_FUNC_COUNT) return -1;
  if (!g_pinsApplied) return PIN_TABLE[f].defaultGpio;
  return g_runtimePins[f];
}

bool outputOn(PinFunc f) {
  int p = pinFor(f);
  if (p < 0) return false;
  return digitalRead(p) == HIGH;
}

void applyPinMap() {
  const BobcatSettings& s = g_settingsManager.getSettings();
  for (int i = 0; i < PIN_FUNC_COUNT; i++) {
    g_runtimePins[i] = s.pinGpio[i];
  }
  g_pinsApplied = true;
}

void loadCalibrationConstants() {
  const BobcatSettings& s = g_settingsManager.getSettings();
  runtime_battery_divider   = s.batteryDivider;
  runtime_temp_scale        = s.tempSensorScale;
  runtime_pressure_scale    = s.oilPressureScale;
  runtime_hyd_pressure_scale= s.hydPressureScale;
  runtime_fuel_empty        = s.fuelLevelEmpty;
  runtime_fuel_full         = s.fuelLevelFull;

  Serial.println("Calibration constants loaded from settings:");
  Serial.printf("  battDiv=%.5f tempScale=%.4f oilScale=%.4f hydScale=%.4f fuel=%d-%d\n",
                runtime_battery_divider, runtime_temp_scale, runtime_pressure_scale,
                runtime_hyd_pressure_scale, runtime_fuel_empty, runtime_fuel_full);
}

void initializePins() {
  Serial.println("Initializing GPIO from runtime pin map...");

  applyPinMap();
  loadCalibrationConstants();

  for (int i = 0; i < PIN_FUNC_COUNT; i++) {
    int gpio = g_runtimePins[i];
    switch (PIN_TABLE[i].type) {
      case PT_RELAY:
      case PT_DOUT:
        pinMode(gpio, OUTPUT);
        digitalWrite(gpio, LOW);   // safe state
        break;
      case PT_DIN:
        pinMode(gpio, INPUT_PULLUP);
        break;
      case PT_ADC:
        // Analog input - no pinMode. Never INPUT_PULLUP (some are input-only).
        break;
    }
    Serial.printf("  %-12s -> GPIO%-2d (%s)\n", PIN_TABLE[i].func, gpio,
                  pinTypeToString(PIN_TABLE[i].type));
  }

  Serial.println("GPIO initialization complete - all relays OFF");
}

// ============================================================================
// OUTPUT CONTROL
// ============================================================================
void controlMainPower(bool enable) {
  digitalWrite(pinFor(PIN_MAIN_POWER), enable ? HIGH : LOW);
  Serial.print("Main Power: "); Serial.println(enable ? "ON" : "OFF");
}

void controlGlowPlugs(bool enable) {
  digitalWrite(pinFor(PIN_GLOW), enable ? HIGH : LOW);
  Serial.print("Glow Plugs: "); Serial.println(enable ? "ON" : "OFF");
}

void controlStarter(bool enable) {
  digitalWrite(pinFor(PIN_STARTER), enable ? HIGH : LOW);
  Serial.print("Starter: "); Serial.println(enable ? "ON" : "OFF");
}

void controlLights(bool enable) {
  digitalWrite(pinFor(PIN_LIGHTS), enable ? HIGH : LOW);
  Serial.print("Lights: "); Serial.println(enable ? "ON" : "OFF");
}

void controlBuzzer(bool enable) {
  digitalWrite(pinFor(PIN_BUZZER), enable ? HIGH : LOW);
}

void controlStatusLed(bool enable) {
  digitalWrite(pinFor(PIN_STATUS_LED), enable ? HIGH : LOW);
}

// ============================================================================
// VIRTUAL BUTTONS (flag/state only - actuation happens in loop())
// ============================================================================
void virtualPowerOnButton() {
  if (g_systemState.keyPosition == 0) {
    g_systemState.keyPosition = 1;
    Serial.println("Web: POWER ON");
  }
}

void virtualPowerOffButton() {
  g_systemState.keyPosition = 0;
  Serial.println("Web: POWER OFF");
}

void virtualStartButton() {
  if (g_systemState.keyPosition < 2) {
    g_systemState.keyPosition = 2; // GLOW first
    Serial.println("Web: START -> GLOW");
  } else {
    g_systemState.keyStartHeld = true;
    g_systemState.keyPosition = 3;
    g_systemState.startHoldTime = millis();
    Serial.println("Web: START -> crank request");
  }
}

void virtualLightsButton() {
  g_systemState.lightsTogglePressed = true;
  Serial.println("Web: Lights toggle");
}

// ============================================================================
// SENSOR READS - one formula per sensor, unified calibration
// ============================================================================
float readEngineTemp() {
  int raw = analogRead(pinFor(PIN_ENGINE_TEMP));
  // Inverted NTC: lower ADC = higher temperature.
  float instant = 150.0f - (raw * runtime_temp_scale);

  if (!tempFilterInitialized) {
    for (int i = 0; i < TEMP_FILTER_SIZE; i++) tempReadings[i] = instant;
    tempFilterInitialized = true;
  }
  tempReadings[tempIndex] = instant;
  tempIndex = (tempIndex + 1) % TEMP_FILTER_SIZE;

  float sum = 0;
  for (int i = 0; i < TEMP_FILTER_SIZE; i++) sum += tempReadings[i];
  return sum / TEMP_FILTER_SIZE;
}

float readOilPressure() {
  int raw = analogRead(pinFor(PIN_OIL_PRESSURE));
  float psi = raw * runtime_pressure_scale;
  if (psi < 0) psi = 0;
  return psi;
}

float readHydraulicPressure() {
  int raw = analogRead(pinFor(PIN_HYDRAULIC));
  float psi = raw * runtime_hyd_pressure_scale;
  if (psi < 0) psi = 0;
  return psi;
}

float readBatteryVoltage() {
  int raw = analogRead(pinFor(PIN_BATTERY));
  return raw * runtime_battery_divider;
}

float readFuelLevel() {
  int raw = analogRead(pinFor(PIN_FUEL));
  long pct = map(raw, runtime_fuel_empty, runtime_fuel_full, 0, 100);
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;   // clamp 0-100
  return (float)pct;
}

// ============================================================================
// SAFETY INTERLOCKS
// ============================================================================
bool readSeatBarSafety() {
  // INPUT_PULLUP: LOW = switch closed = operator seated.
  return digitalRead(pinFor(PIN_SEAT_BAR)) == LOW;
}

bool readNeutralSafety() {
  // INPUT_PULLUP: LOW = switch closed = transmission in neutral.
  return digitalRead(pinFor(PIN_NEUTRAL)) == LOW;
}

bool safetyInterlocksPassed() {
  bool seated = readSeatBarSafety();
  bool neutral = readNeutralSafety();
  if (!seated)  Serial.println("SAFETY: operator not seated (seat bar open)");
  if (!neutral) Serial.println("SAFETY: transmission not in neutral");
  return seated && neutral;
}

bool batteryOkToStart() {
  float v = readBatteryVoltage();
  float minV = g_settingsManager.getMinBatteryVoltage();
  if (v < minV) {
    Serial.printf("SAFETY: battery too low to crank (%.1fV < %.1fV)\n", v, minV);
    return false;
  }
  return true;
}

// ============================================================================
// ENGINE STATE HELPERS
// ============================================================================
bool isEngineRunning() {
  int st = g_systemState.currentState;
  return (st == RUNNING || st == LOW_OIL_PRESSURE || st == HIGH_TEMPERATURE);
}

void performSafetyShutdown() {
  controlGlowPlugs(false);
  controlStarter(false);
  Serial.println("Safety shutdown performed (starter + glow off)");
}

// ============================================================================
// POWER MANAGEMENT
// ============================================================================
void initializeSleepMode() {
  Serial.println("Initializing deep sleep mode...");
  pinMode(WAKE_UP_BUTTON_PIN, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0); // wake on BOOT button LOW

  g_systemState.lastActivityTime = millis();
  g_systemState.sleepModeEnabled = true;
  g_systemState.wakeUpPending = false;
  g_systemState.sleepTimer = millis();
  Serial.println("Deep sleep initialized - wake on GPIO0");
}

void enterDeepSleep() {
  Serial.println("Preparing for deep sleep...");
  prepareForSleep();
  Serial.println("Entering deep sleep (wake with BOOT button GPIO0)");
  Serial.flush();
  esp_deep_sleep_start();
}

void prepareForSleep() {
  controlMainPower(false);
  controlGlowPlugs(false);
  controlStarter(false);
  controlLights(false);
  controlBuzzer(false);
  controlStatusLed(false);
  g_systemState.keyPosition = 0;
  g_systemState.currentState = OFF;
  Serial.println("System prepared for sleep - all outputs OFF");
}

bool checkSleepConditions() {
  return checkSleepConditions(false);
}

bool checkSleepConditions(bool manualSleep) {
  if (isEngineRunning()) return false;
  if (g_systemState.currentState == ERROR ||
      g_systemState.currentState == HIGH_TEMPERATURE ||
      g_systemState.currentState == LOW_OIL_PRESSURE) return false;
  if (g_systemState.keyPosition != 0) return false;
  if (!g_systemState.sleepModeEnabled) return false;
  if (manualSleep) return true;

  unsigned long timeSinceActivity = millis() - g_systemState.lastActivityTime;
  return (timeSinceActivity >= ACTIVITY_TIMEOUT);
}

void handleWakeUp() {
  Serial.println("=== WAKE UP FROM DEEP SLEEP ===");
  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();
  switch (reason) {
    case ESP_SLEEP_WAKEUP_EXT0:  Serial.println("Woke by button"); break;
    case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Woke by timer"); break;
    default:                     Serial.println("Woke (unknown)"); break;
  }
  g_systemState.keyPosition = 0;
  g_systemState.currentState = OFF;
  g_systemState.wakeUpPending = false;
  g_systemState.lastActivityTime = millis();
  Serial.println("Ready after wake-up");
}

void updateActivityTimer() {
  g_systemState.lastActivityTime = millis();
}
