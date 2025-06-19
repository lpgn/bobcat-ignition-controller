/*
 * Hardware Control Implementation for Bobcat Ignition Controller
 * Implements all hardware control functions
 */

#include "hardware.h"
#include "config.h"

void initializePins() {
  // Initialize output pins for relays
  pinMode(GLOW_PLUGS_PIN, OUTPUT);
  pinMode(IGNITION_PIN, OUTPUT);
  pinMode(STARTER_PIN, OUTPUT);

  // Initialize digital input pins
  pinMode(ENGINE_RUN_FEEDBACK_PIN, INPUT_PULLUP);
  pinMode(ALTERNATOR_CHARGE_PIN, INPUT_PULLUP);

  // ADC pins for sensors (no pinMode needed for ADC)

  // Initialize all outputs to safe state
  digitalWrite(GLOW_PLUGS_PIN, LOW);
  digitalWrite(IGNITION_PIN, LOW);
  digitalWrite(STARTER_PIN, LOW);
}

void controlGlowPlugs(bool enable) {
  digitalWrite(GLOW_PLUGS_PIN, enable ? HIGH : LOW);
}

void controlIgnition(bool enable) {
  digitalWrite(IGNITION_PIN, enable ? HIGH : LOW);
}

void controlStarter(bool enable) {
  digitalWrite(STARTER_PIN, enable ? HIGH : LOW);
}

// Virtual button functions for web interface - Tesla-style fly-by-wire control
void virtualStartButton() {
  if (!startButtonPressed) {
    startButtonPressed = true;
    Serial.println("Web Interface: START button pressed");
  }
}

void virtualStopButton() {
  if (!stopButtonPressed) {
    stopButtonPressed = true;
    Serial.println("Web Interface: STOP button pressed");
  }
}

// Sensor reading functions with proper calibration
float readEngineTemp() {
  int rawValue = analogRead(ENGINE_TEMP_PIN);
  return (rawValue * TEMP_SENSOR_SCALE) + TEMP_SENSOR_OFFSET;
}

float readOilPressure() {
  int rawValue = analogRead(OIL_PRESSURE_PIN);
  return (rawValue * OIL_PRESSURE_SCALE) + OIL_PRESSURE_OFFSET;
}

float readBatteryVoltage() {
  int rawValue = analogRead(BATTERY_VOLTAGE_PIN);
  return rawValue * BATTERY_VOLTAGE_DIVIDER;
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
  return digitalRead(ENGINE_RUN_FEEDBACK_PIN) == HIGH;
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
  // Example: turn off all relays
  controlGlowPlugs(false);
  controlIgnition(false);
  controlStarter(false);
  Serial.println("Safety shutdown performed!");
}
