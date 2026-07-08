/*
 * Safety Monitoring Implementation for Bobcat Ignition Controller
 * Alerts only - a diesel is stopped by its fuel lever, never by this controller.
 */

#include "safety.h"
#include "config.h"
#include "hardware.h"
#include "system_state.h"
#include "settings.h"

void checkSafetyInputs() {
  // Unified low-battery check (single threshold: settings.minBatteryVoltage).
  if (g_systemState.currentState == OFF) return;
  float volts = readBatteryVoltage();
  float minV = g_settingsManager.getMinBatteryVoltage();
  if (volts < minV) {
    handleError("Low battery voltage");
  }
}

void checkEngineVitals() {
  int st = g_systemState.currentState;
  // Only meaningful while the engine is (supposed to be) running.
  if (st != RUNNING && st != LOW_OIL_PRESSURE && st != HIGH_TEMPERATURE) return;

  const BobcatSettings& s = g_settingsManager.getSettings();
  float temp = readEngineTemp();
  float oil = readOilPressure();
  bool over = temp > s.maxCoolantTemp;
  bool lowOil = oil < s.minOilPressure;

  if (over) {
    if (st != HIGH_TEMPERATURE) {
      g_systemState.currentState = HIGH_TEMPERATURE;
      handleError("Engine temperature too high");
    }
  } else if (lowOil) {
    if (st != LOW_OIL_PRESSURE) {
      g_systemState.currentState = LOW_OIL_PRESSURE;
      handleError("Oil pressure too low");
    }
  } else {
    // Conditions clear - return to normal RUNNING.
    if (st != RUNNING) {
      g_systemState.currentState = RUNNING;
      Serial.println("Engine vitals recovered - RUNNING");
    }
  }
}

void handleError(const char* errorMessage) {
  Serial.print("ALERT: ");
  Serial.println(errorMessage);
  // Alert only. The engine must be stopped manually with the fuel lever.
}

// Deliberate override crank. Runs in loop() (triggered by the overrideRequested
// flag set from POST /api/control). Bypasses SENSOR fault states, but interlocks
// and the battery-OK check are STILL enforced.
void overrideStart() {
  Serial.println("OVERRIDE: deliberate crank requested (bypassing sensor faults)");

  if (!g_systemState.maintenanceMode) {
    if (!safetyInterlocksPassed()) {
      Serial.println("OVERRIDE BLOCKED: safety interlocks not satisfied");
      return;
    }
    if (!batteryOkToStart()) {
      Serial.println("OVERRIDE BLOCKED: battery voltage too low");
      return;
    }
  }

  unsigned long now = millis();
  controlMainPower(true);
  controlGlowPlugs(true);
  controlStarter(true);
  g_systemState.currentState = START;
  g_systemState.ignitionStartTime = now;
  g_systemState.startHoldTime = now;
  g_systemState.keyStartHeld = true;
  g_systemState.keyPosition = 3;
  g_systemState.oilOkSince = 0;
  if (g_systemState.glowPlugStartTime == 0) g_systemState.glowPlugStartTime = now;
  Serial.println("OVERRIDE: starter engaged");
}
