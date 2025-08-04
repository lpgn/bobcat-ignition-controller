/*
 * Safety Monitoring Implementation for Bobcat Ignition Controller
 * Implements all safety checks and error handling
 */

#include "safety.h"
#include "config.h"
#include "hardware.h"
#include "system_state.h"

void checkSafetyInputs() {
  // Read analog sensors
  int batteryVoltage = analogRead(BATTERY_VOLTAGE_PIN);
  int engineTemp = analogRead(ENGINE_TEMP_PIN);
  int oilPressure = analogRead(OIL_PRESSURE_PIN);
  
  // Convert ADC readings to meaningful values
  // Battery voltage: assuming voltage divider for 12V system
  float batteryVolts = (batteryVoltage * 3.3 / 4095.0) * (12.0 / 3.3); // Adjust divider ratio as needed
  
  // Check for low battery voltage
  if (batteryVolts < 10.5 && g_systemState.currentState != OFF) {
    handleError("Low battery voltage");
    return;
  }
  
  // Add more safety checks as needed
  // - Engine temperature limits
  // - Oil pressure monitoring
  // - Emergency stop conditions
}

void checkEngineVitals() {
  if (g_systemState.currentState != RUNNING) {
    return; // Only check vitals when the engine is supposed to be running
  }

  // Read and convert sensor values
  int tempReading = analogRead(ENGINE_TEMP_PIN);
  float engineTemp = (tempReading * TEMP_SENSOR_SCALE) + TEMP_SENSOR_OFFSET;

  int pressureReading = analogRead(OIL_PRESSURE_PIN);
  float oilPressure = (pressureReading * OIL_PRESSURE_SCALE) + OIL_PRESSURE_OFFSET;

  // Check for high temperature
  if (engineTemp > MAX_COOLANT_TEMP) {
    g_systemState.currentState = HIGH_TEMPERATURE;
    handleError("Engine temperature too high!");
  }

  // Check for low oil pressure
  if (oilPressure < MIN_OIL_PRESSURE) {
    g_systemState.currentState = LOW_OIL_PRESSURE;
    handleError("Oil pressure too low!");
  }
}

void handleError(const char* errorMessage) {
  Serial.print("ALERT: ");
  Serial.println(errorMessage);
  
  // Don't shut down the engine, just set alert state
  // The engine must be manually shut down with the lever
}

void overrideStart() {
    Serial.println("OVERRIDE: Bypassing safety checks and starting engine!");
    
    // Make sure main power is on
    controlMainPower(true);
    
    // Set state and start cranking
    g_systemState.currentState = START;
    g_systemState.ignitionStartTime = millis();
    g_systemState.keyStartHeld = true;
    g_systemState.keyPosition = 3;
    g_systemState.startHoldTime = millis();
    controlStarter(true);
    
    Serial.println("OVERRIDE: Starter engaged");
}
