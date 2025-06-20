/*
 * Hardware Control Implementation for Bobcat Ignition Controller
 * Implements all hardware control functions
 */

#include "hardware.h"
#include "config.h"

void initializePins() {
  // Initialize output pins for relays
  pinMode(MAIN_POWER_PIN, OUTPUT);
  pinMode(GLOW_PLUGS_PIN, OUTPUT);
  pinMode(STARTER_PIN, OUTPUT);
  pinMode(FRONT_LIGHT_PIN, OUTPUT);
  pinMode(BACK_LIGHT_PIN, OUTPUT);

  // Initialize digital input pins
  pinMode(ENGINE_RUN_FEEDBACK_PIN, INPUT_PULLUP);
  pinMode(ALTERNATOR_CHARGE_PIN, INPUT_PULLUP);

  // ADC pins for sensors (no pinMode needed for ADC)

  // Initialize all outputs to safe state
  digitalWrite(MAIN_POWER_PIN, LOW);
  digitalWrite(GLOW_PLUGS_PIN, LOW);
  digitalWrite(STARTER_PIN, LOW);
  digitalWrite(FRONT_LIGHT_PIN, LOW);
  digitalWrite(BACK_LIGHT_PIN, LOW);
}

void controlMainPower(bool enable) {
    digitalWrite(MAIN_POWER_PIN, enable ? HIGH : LOW);
}

void controlGlowPlugs(bool enable) {
  digitalWrite(GLOW_PLUGS_PIN, enable ? HIGH : LOW);
}

// controlIgnition function removed - not used in this Bobcat model

void controlStarter(bool enable) {
  digitalWrite(STARTER_PIN, enable ? HIGH : LOW);
}

void controlFrontLight(bool enable) {
    digitalWrite(FRONT_LIGHT_PIN, enable ? HIGH : LOW);
}

void controlBackLight(bool enable) {
    digitalWrite(BACK_LIGHT_PIN, enable ? HIGH : LOW);
}

// Virtual button functions for web interface - Tesla-style fly-by-wire control
void virtualPowerOnButton() {
    if (!powerOnButtonPressed) {
        powerOnButtonPressed = true;
        Serial.println("Web Interface: POWER ON button pressed");
    }
}

void virtualPowerOffButton() {
    if (!powerOffButtonPressed) {
        powerOffButtonPressed = true;
        Serial.println("Web Interface: POWER OFF button pressed");
    }
}

void virtualStartButton() {
  if (!startButtonPressed) {
    startButtonPressed = true;
    Serial.println("Web Interface: START button pressed");
  }
}

void virtualFrontLightButton() {
    static bool frontLightOn = false;
    frontLightOn = !frontLightOn;
    controlFrontLight(frontLightOn);
    Serial.println(frontLightOn ? "Web Interface: Front light ON" : "Web Interface: Front light OFF");
}

void virtualBackLightButton() {
    static bool backLightOn = false;
    backLightOn = !backLightOn;
    controlBackLight(backLightOn);
    Serial.println(backLightOn ? "Web Interface: Back light ON" : "Web Interface: Back light OFF");
}

// Note: No virtualStopButton - engine must be stopped manually with lever

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
  // Only turn off glow plugs and starter - no ignition relay in this model
  controlGlowPlugs(false);
  controlStarter(false);
  Serial.println("Safety shutdown performed!");
}
