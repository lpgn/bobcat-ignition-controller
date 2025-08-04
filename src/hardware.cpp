/*
 * Hardware Control Implementation for Bobcat Ignition Controller
 * Implements all hardware control functions
 */

#include "hardware.h"
#include "config.h"

void initializePins() {
  Serial.println("Initializing GPIO pins...");
  
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
    if (keyPosition == 0) {
        keyPosition = 1;
        Serial.println("Web Interface: POWER ON button pressed");
    }
}

void virtualPowerOffButton() {
    // Set key position to OFF
    keyPosition = 0;
    Serial.println("Web Interface: POWER OFF button pressed");
}

void virtualStartButton() {
    // Legacy start button - simulate turning key to GLOW position then START
    if (keyPosition < 2) {
        keyPosition = 2; // Move to GLOW position first
        Serial.println("Web Interface: START button pressed - moving to GLOW position");
    } else {
        keyStartHeld = true;
        keyPosition = 3;
        startHoldTime = millis();
        Serial.println("Web Interface: START button pressed - cranking");
    }
}

void virtualLightsButton() {
    lightsTogglePressed = true;
    Serial.println("Web Interface: Lights toggle pressed");
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
