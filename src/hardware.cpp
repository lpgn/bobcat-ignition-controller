/*
 * Hardware Control Implementation for Bobcat Ignition Controller
 * Implements all hardware control functions
 */

#include "hardware.h"
#include "config.h"
#include "system_state.h"
#include <Preferences.h>

// Runtime calibration constants (loaded from preferences)
float runtime_battery_divider = BATTERY_VOLTAGE_DIVIDER;
float runtime_temp_scale = TEMP_SENSOR_SCALE;
float runtime_pressure_scale = OIL_PRESSURE_SCALE;

void initializePins() {
  Serial.println("Initializing GPIO pins...");
  
  // Load calibration constants from preferences
  loadCalibrationConstants();
  
  // Initialize output pins for relays
  pinMode(MAIN_POWER_PIN, OUTPUT);
  pinMode(GLOW_PLUGS_PIN, OUTPUT);
  pinMode(STARTER_PIN, OUTPUT);
  pinMode(LIGHTS_PIN, OUTPUT);

  Serial.print("Relay pins configured: ");
  Serial.print("Main Power (GPIO"); Serial.print(MAIN_POWER_PIN); Serial.print("), ");
  Serial.print("Glow Plugs (GPIO"); Serial.print(GLOW_PLUGS_PIN); Serial.print("), ");
  Serial.print("Starter (GPIO"); Serial.print(STARTER_PIN); Serial.print("), ");
  Serial.print("Lights (GPIO"); Serial.print(LIGHTS_PIN); Serial.println(")");

  // Initialize digital input pins
  pinMode(ENGINE_RUN_FEEDBACK_PIN, INPUT_PULLUP);
  pinMode(ALTERNATOR_CHARGE_PIN, INPUT_PULLUP);

  // ADC pins for sensors (no pinMode needed for ADC)

  // Initialize all outputs to safe state
  digitalWrite(MAIN_POWER_PIN, LOW);
  digitalWrite(GLOW_PLUGS_PIN, LOW);
  digitalWrite(STARTER_PIN, LOW);
  digitalWrite(LIGHTS_PIN, LOW);
  
  Serial.println("All relays initialized to OFF state");
  Serial.println("GPIO initialization complete");
}

void loadCalibrationConstants() {
  Preferences prefs;
  prefs.begin("calibration", true); // Open in read-only mode
  
  // Load constants with defaults from config
  runtime_battery_divider = prefs.getFloat("battery_div", BATTERY_VOLTAGE_DIVIDER);
  runtime_temp_scale = prefs.getFloat("temp_scale", TEMP_SENSOR_SCALE);
  runtime_pressure_scale = prefs.getFloat("pressure_scale", OIL_PRESSURE_SCALE);
  
  prefs.end();
  
  Serial.println("Calibration constants loaded:");
  Serial.print("  Battery divider: "); Serial.println(runtime_battery_divider, 6);
  Serial.print("  Temperature scale: "); Serial.println(runtime_temp_scale, 6);
  Serial.print("  Pressure scale: "); Serial.println(runtime_pressure_scale, 6);
}

void controlMainPower(bool enable) {
    digitalWrite(MAIN_POWER_PIN, enable ? HIGH : LOW);
    Serial.print("Main Power: ");
    Serial.println(enable ? "ON" : "OFF");
}

void controlGlowPlugs(bool enable) {
  digitalWrite(GLOW_PLUGS_PIN, enable ? HIGH : LOW);
  Serial.print("Glow Plugs: ");
  Serial.println(enable ? "ON" : "OFF");
}

// controlIgnition function removed - not used in this Bobcat model

void controlStarter(bool enable) {
  digitalWrite(STARTER_PIN, enable ? HIGH : LOW);
  Serial.print("Starter: ");
  Serial.println(enable ? "ON" : "OFF");
}

void controlLights(bool enable) {
    digitalWrite(LIGHTS_PIN, enable ? HIGH : LOW);
    Serial.print("Lights: ");
    Serial.println(enable ? "ON" : "OFF");
}

// Virtual button functions for web interface - Tesla-style fly-by-wire control
void virtualPowerOnButton() {
    // Set key position to ON
    if (g_systemState.keyPosition == 0) {
        g_systemState.keyPosition = 1;
        Serial.println("Web Interface: POWER ON button pressed");
    }
}

void virtualPowerOffButton() {
    // Set key position to OFF
    g_systemState.keyPosition = 0;
    Serial.println("Web Interface: POWER OFF button pressed");
}

void virtualStartButton() {
    // Legacy start button - simulate turning key to GLOW position then START
    if (g_systemState.keyPosition < 2) {
        g_systemState.keyPosition = 2; // Move to GLOW position first
        Serial.println("Web Interface: START button pressed - moving to GLOW position");
    } else {
        g_systemState.keyStartHeld = true;
        g_systemState.keyPosition = 3;
        g_systemState.startHoldTime = millis();
        Serial.println("Web Interface: START button pressed - cranking");
    }
}

void virtualLightsButton() {
    g_systemState.lightsTogglePressed = true;
    Serial.println("Web Interface: Lights toggle pressed");
}

// Note: No virtualStopButton - engine must be stopped manually with lever

// Sensor reading functions with proper calibration
float readEngineTemp() {
  int rawValue = analogRead(ENGINE_TEMP_PIN);
  return (rawValue * runtime_temp_scale) + TEMP_SENSOR_OFFSET;
}

float readOilPressure() {
  int rawValue = analogRead(OIL_PRESSURE_PIN);
  return (rawValue * runtime_pressure_scale) + OIL_PRESSURE_OFFSET;
}

float readBatteryVoltage() {
  int rawValue = analogRead(BATTERY_VOLTAGE_PIN);
  return rawValue * runtime_battery_divider;
}

float readFuelLevel() {
  int rawValue = analogRead(FUEL_LEVEL_PIN);
  // Convert to percentage (0-100%)
  return map(rawValue, FUEL_LEVEL_EMPTY, FUEL_LEVEL_FULL, 0, 100);
}

// Digital input reading functions
bool readAlternatorCharge() {
  // Active LOW: 0 = charging, 1 = not charging
  return digitalRead(ALTERNATOR_CHARGE_PIN) == LOW;
}

bool readEngineRunFeedback() {
  // When no sensor is connected, floating pin can read HIGH
  // For development/testing with no sensors: assume engine is OFF
  // TODO: Remove this override when sensors are connected
  return false; // Override: assume engine OFF when no sensors connected
  
  // Original logic (uncomment when sensors are connected):
  // return digitalRead(ENGINE_RUN_FEEDBACK_PIN) == HIGH;
}

// Safety check functions (basic stubs, expand as needed)
bool checkEngineSafetyConditions() {
  // Example: always safe for now
  return true;
}

bool isEngineRunning() {
  // Use engine run feedback pin for now
  return readEngineRunFeedback();
}

void performSafetyShutdown() {
  // Only turn off glow plugs and starter - no ignition relay in this model
  controlGlowPlugs(false);
  controlStarter(false);
  Serial.println("Safety shutdown performed!");
}

// ============================================================================
// POWER MANAGEMENT FUNCTIONS
// ============================================================================

void initializeSleepMode() {
  Serial.println("Initializing deep sleep mode...");
  
  // Configure wake-up button
  pinMode(WAKE_UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SLEEP_ENABLE_PIN, INPUT_PULLUP);
  
  // Configure wake-up sources
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0); // Wake on button press (LOW)
  
  // Initialize sleep-related state
  g_systemState.lastActivityTime = millis();
  g_systemState.sleepModeEnabled = true;
  g_systemState.wakeUpPending = false;
  g_systemState.sleepTimer = millis();
  
  Serial.println("Deep sleep mode initialized - Wake up button: GPIO0");
}

void enterDeepSleep() {
  Serial.println("Preparing for deep sleep...");
  
  // Prepare system for sleep
  prepareForSleep();
  
  // Print wake-up information
  Serial.println("Entering deep sleep mode...");
  Serial.println("Wake up by pressing the BOOT button (GPIO0)");
  Serial.flush(); // Ensure all serial output is sent
  
  // Enter deep sleep
  esp_deep_sleep_start();
}

void prepareForSleep() {
  Serial.println("Preparing system for sleep...");
  
  // Turn off all relays to save power
  controlMainPower(false);
  controlGlowPlugs(false);
  controlStarter(false);
  controlLights(false);
  
  // Save current state (could save to RTC memory if needed)
  // For now, we'll just reset to OFF state on wake-up
  g_systemState.keyPosition = 0; // OFF
  g_systemState.currentState = OFF;
  
  Serial.println("System prepared for sleep - all relays OFF");
}

bool checkSleepConditions() {
  return checkSleepConditions(false); // Default to automatic sleep check
}

bool checkSleepConditions(bool manualSleep) {
  // Don't sleep if engine is running
  if (isEngineRunning()) {
    return false;
  }
  
  // Don't sleep if system is in critical state
  if (g_systemState.currentState == ERROR || 
      g_systemState.currentState == HIGH_TEMPERATURE ||
      g_systemState.currentState == LOW_OIL_PRESSURE) {
    return false;
  }
  
  // Don't sleep if key is not in OFF position
  if (g_systemState.keyPosition != 0) {
    return false;
  }
  
  // Don't sleep if sleep mode is disabled
  if (!g_systemState.sleepModeEnabled) {
    return false;
  }
  
  // For manual sleep, skip the activity timeout check
  if (manualSleep) {
    return true;
  }
  
  // For automatic sleep, check if enough time has passed since last activity
  unsigned long currentTime = millis();
  unsigned long timeSinceActivity = currentTime - g_systemState.lastActivityTime;
  
  return (timeSinceActivity >= ACTIVITY_TIMEOUT);
}

void handleWakeUp() {
  Serial.println("=== WAKE UP FROM DEEP SLEEP ===");
  
  // Check wake-up reason
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Woke up by external signal (button press)");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Woke up by timer");
      break;
    default:
      Serial.println("Woke up for unknown reason");
      break;
  }
  
  // Reset system state after wake-up
  g_systemState.keyPosition = 0; // OFF
  g_systemState.currentState = OFF;
  g_systemState.wakeUpPending = false;
  g_systemState.lastActivityTime = millis();
  
  // Reinitialize hardware
  initializePins();
  
  Serial.println("System ready after wake-up");
}

void updateActivityTimer() {
  g_systemState.lastActivityTime = millis();
}
