/*
 * System State Management Implementation for Bobcat Ignition Controller
 * Implements the main state machine logic
 */

#include "system_state.h"
#include "config.h"
#include "hardware.h"
#include "safety.h"

void runIgnitionSequence() {
  switch (currentState) {
    case IDLE:
      if (powerOnButtonPressed) {
        Serial.println("Power ON - System energized");
        currentState = POWER_ON;
        controlMainPower(true);
        powerOnButtonPressed = false;
      }
      break;
      
    case POWER_ON:
      if (powerOffButtonPressed) {
        Serial.println("Power OFF - System shutdown");
        currentState = IDLE;
        controlMainPower(false);
        controlLights(false);
        powerOffButtonPressed = false;
      } else if (startButtonPressed) {
        Serial.println("Starting ignition sequence...");
        Serial.println("Phase 1: Glow plug heating (20 seconds)");
        currentState = GLOW_PLUG_HEATING;
        glowPlugStartTime = millis();
        controlGlowPlugs(true);
        startButtonPressed = false;
      }
      break;
      
    case GLOW_PLUG_HEATING:
      if (powerOffButtonPressed) {
        Serial.println("Ignition sequence aborted");
        currentState = IDLE;
        controlMainPower(false);
        controlGlowPlugs(false);
        powerOffButtonPressed = false;
      } else if (millis() - glowPlugStartTime >= GLOW_PLUG_DURATION) {
        Serial.println("Glow plug heating complete");
        Serial.println("Ready to start engine - Press START again to crank");
        currentState = READY_TO_START;
        controlGlowPlugs(false);
      }
      break;
      
    case READY_TO_START:
      if (powerOffButtonPressed) {
        Serial.println("Start sequence cancelled");
        currentState = IDLE;
        controlMainPower(false);
        powerOffButtonPressed = false;
      } else if (startButtonPressed) {
        Serial.println("Phase 2: Engine cranking");
        currentState = STARTING;
        ignitionStartTime = millis();
        controlStarter(true);
        startButtonPressed = false;
      }
      // Timeout after 30 seconds of waiting
      else if (millis() - glowPlugStartTime >= (GLOW_PLUG_DURATION + 30000)) {
        Serial.println("Start timeout - returning to power on state");
        currentState = POWER_ON;
      }
      break;
      
    case STARTING:
      if (powerOffButtonPressed) {
        Serial.println("Engine start aborted");
        currentState = IDLE;
        controlMainPower(false);
        controlStarter(false);
        powerOffButtonPressed = false;
      } else if (millis() - ignitionStartTime >= IGNITION_TIMEOUT) {
        Serial.println("Engine start timeout - stopping cranking");
        controlStarter(false);
        currentState = RUNNING; // Assume engine started
      }
      break;
      
    case RUNNING:
      if (powerOffButtonPressed) {
        Serial.println("Power shutdown requested - engine must be stopped manually");
        currentState = IDLE;
        controlMainPower(false);
        powerOffButtonPressed = false;
      }
      // Monitor engine parameters here
      break;
      
    case LOW_OIL_PRESSURE:
    case HIGH_TEMPERATURE:
      // Alert states - continue monitoring but show warning
      if (powerOffButtonPressed) {
        Serial.println("Power shutdown during alert state");
        currentState = IDLE;
        controlMainPower(false);
        powerOffButtonPressed = false;
      }
      break;
      
    case ERROR:
      // Handle error state - status will be shown in web interface
      if (powerOffButtonPressed) {
        currentState = IDLE;
        controlMainPower(false);
        powerOffButtonPressed = false;
      }
      break;
  }

  // Reset button flags after processing
  startButtonPressed = false;
  powerOnButtonPressed = false;
  powerOffButtonPressed = false;
}
