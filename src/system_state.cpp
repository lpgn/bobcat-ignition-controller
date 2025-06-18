/*
 * System State Management Implementation for Bobcat Ignition Controller
 * Implements the main state machine logic
 */

#include "system_state.h"
#include "config.h"
#include "hardware.h"
#include "safety.h"

void updateSystem() {
  switch (currentState) {
    case IDLE:
      if (startButtonPressed) {
        Serial.println("Starting ignition sequence...");
        Serial.println("Phase 1: Glow plug heating (20 seconds)");
        currentState = GLOW_PLUG_HEATING;
        glowPlugStartTime = millis();
        controlGlowPlugs(true);
      }
      break;
      
    case GLOW_PLUG_HEATING:
      if (stopButtonPressed) {
        Serial.println("Ignition sequence aborted");
        currentState = SHUTDOWN;
      } else if (millis() - glowPlugStartTime >= GLOW_PLUG_DURATION) {
        Serial.println("Glow plug heating complete");
        Serial.println("Ready to start engine - Press START again to crank");
        currentState = READY_TO_START;
        controlGlowPlugs(false);
      }
      break;
      
    case READY_TO_START:
      if (stopButtonPressed) {
        Serial.println("Start sequence cancelled");
        currentState = SHUTDOWN;
      } else if (startButtonPressed) {
        Serial.println("Phase 2: Engine cranking");
        currentState = STARTING;
        ignitionStartTime = millis();
        controlIgnition(true);
      }
      // Timeout after 30 seconds of waiting
      else if (millis() - glowPlugStartTime >= (GLOW_PLUG_DURATION + 30000)) {
        Serial.println("Start timeout - returning to idle");
        currentState = IDLE;
      }
      break;
      
    case STARTING:
      if (stopButtonPressed) {
        Serial.println("Engine start aborted");
        currentState = SHUTDOWN;
      } else if (millis() - ignitionStartTime >= IGNITION_TIMEOUT) {
        Serial.println("Engine start timeout - stopping cranking");
        controlIgnition(false);
        currentState = RUNNING; // Assume engine started or ready to retry
      }
      // In a real implementation, you'd check engine RPM or oil pressure
      // to determine if engine actually started
      break;
      
    case RUNNING:
      if (stopButtonPressed) {
        Serial.println("Engine shutdown requested");
        currentState = SHUTDOWN;
      }
      // Monitor engine parameters here
      break;
      
    case SHUTDOWN:
      if (!shutdownInProgress) {
        Serial.println("Shutting down system...");
        controlGlowPlugs(false);
        controlIgnition(false);
        shutdownStartTime = millis();
        shutdownInProgress = true;
      } else if (millis() - shutdownStartTime >= 1000) {
        // Brief delay for relay settling completed
        currentState = IDLE;
        shutdownInProgress = false;
        Serial.println("System ready for next start cycle");
      }
      break;
      
    case ERROR:
      // Handle error state - status will be shown in web interface
      if (stopButtonPressed) {
        currentState = SHUTDOWN;
      }
      break;
  }
}
