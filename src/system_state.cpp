/*
 * System State Management Implementation for Bobcat Ignition Controller
 * Implements the main state machine logic
 */

#include "system_state.h"
#include "config.h"
#include "hardware.h"
#include "safety.h"

/*
 * System State Management Implementation for Bobcat Ignition Controller
 * Implements proper ignition key sequence: OFF -> ON -> GLOW_PLUG -> START (momentary)
 */

#include "system_state.h"
#include "config.h"
#include "hardware.h"
#include "safety.h"

void runIgnitionSequence() {
  static SystemState lastState = (SystemState)-1; // Track state changes
  static int lastKeyPosition = -1; // Track key position changes
  
  // Only execute state logic when state or key position changes
  bool stateChanged = (currentState != lastState);
  bool keyChanged = (keyPosition != lastKeyPosition);
  
  // Handle emergency stop (always process immediately)
  if (emergencyStopPressed) {
    Serial.println("EMERGENCY STOP activated - stopping engine processes only");
    // Stop engine processes but keep main power for lights
    controlGlowPlugs(false);
    controlStarter(false);
    // Keep main power ON so lights can still work
    // Keep work lights in their current state
    keyPosition = 1;  // Force key to ON position (not OFF) to maintain main power
    if (currentState != ON) {
      currentState = ON;  // Return to ON state, not OFF
      stateChanged = true;
    }
    emergencyStopPressed = false;
  }

  // Handle lights toggle (independent of engine state - always process)
  if (lightsTogglePressed) {
    workLightsOn = !workLightsOn;
    controlLights(workLightsOn);
    Serial.println(workLightsOn ? "Work lights ON" : "Work lights OFF");
    lightsTogglePressed = false;
  }
  
  if (!stateChanged && !keyChanged) {
    // Handle time-based logic for glow plugs (works in any state)
    if (currentState == GLOW_PLUG || currentState == START || currentState == RUNNING) {
      if (millis() - glowPlugStartTime >= GLOW_PLUG_DURATION) {
        // Glow plug heating complete - turn off glow plugs
        controlGlowPlugs(false);
        if (currentState == GLOW_PLUG) {
          Serial.println("Glow plug heating complete - ready for start");
        }
      }
    }
    return; // No state change, no need to execute switch statement
  }
  
  // Update tracking variables
  lastState = currentState;
  lastKeyPosition = keyPosition;
  
  // Handle key position changes and automatic state transitions
  switch (currentState) {
    case OFF:
      // Key OFF - Turn off ALL power including lights (master power off)
      if (stateChanged) {
        controlMainPower(false);
        controlGlowPlugs(false);
        controlStarter(false);
        controlLights(false);  // OFF turns off everything including lights
        workLightsOn = false;  // Reset work lights state
        Serial.println("KEY OFF - System shutdown");
      }
      
      if (keyPosition >= 1) {  // Key turned to ON position or beyond
        Serial.println("KEY ON - System energized");
        currentState = ON;
        controlMainPower(true);
      }
      break;
      
    case ON:
      // Key ON - Basic electrical systems active
      if (keyPosition == 0) {  // Key turned back to OFF
        Serial.println("KEY OFF - System shutdown");
        currentState = OFF;
      } else if (keyPosition >= 3 || keyStartHeld) {  // Direct START from ON (auto-glow)
        Serial.println("Direct START - Auto-activating glow plugs and cranking");
        currentState = START;
        glowPlugStartTime = millis();  // Start glow plug timer
        ignitionStartTime = millis();
        startHoldTime = millis();
        controlGlowPlugs(true);  // Auto-activate glow plugs for direct start
        controlStarter(true);    // Start cranking
      } else if (keyPosition >= 2) {  // Key turned to GLOW_PLUG position
        Serial.println("GLOW PLUG position - Starting glow plug heating (20 seconds)");
        currentState = GLOW_PLUG;
        glowPlugStartTime = millis();
        controlGlowPlugs(true);
      }
      break;
      
    case GLOW_PLUG:
      // Glow plug heating phase
      if (keyPosition == 0) {  // Key turned to OFF
        Serial.println("KEY OFF during glow plug heating");
        currentState = OFF;
        controlGlowPlugs(false);
      } else if (keyPosition == 1) {  // Key returned to ON
        Serial.println("Key returned to ON - stopping glow plug heating");
        currentState = ON;
        controlGlowPlugs(false);
      } else if (keyPosition >= 3 || keyStartHeld) {  // Key turned to START
        Serial.println("START position - Engine cranking");
        currentState = START;
        ignitionStartTime = millis();
        startHoldTime = millis();
        // Keep glow plugs ON during start for full duration - some engines need this
        controlStarter(true);     // Start cranking
      } else if (millis() - glowPlugStartTime >= GLOW_PLUG_DURATION) {
        // Glow plug heating complete - automatically move to ready state
        Serial.println("Glow plug heating complete - ready for start");
        // Stay in GLOW_PLUG state but turn off the glow plugs
        controlGlowPlugs(false);
        
        // Show countdown every 2 seconds during heating
        static unsigned long lastCountdown = 0;
        if (millis() - lastCountdown >= 2000) {
          unsigned long elapsed = millis() - glowPlugStartTime;
          unsigned long remaining = (GLOW_PLUG_DURATION - elapsed) / 1000;
          if (remaining > 0) {
            Serial.print("Glow plug heating... ");
            Serial.print(remaining);
            Serial.println(" seconds remaining");
          }
          lastCountdown = millis();
        }
      }
      break;
      
    case START:
      // Engine cranking phase
      // Turn off glow plugs if duration has expired
      if (millis() - glowPlugStartTime >= GLOW_PLUG_DURATION) {
        controlGlowPlugs(false);
      }
      
      if (keyPosition == 0) {  // Key turned to OFF
        Serial.println("KEY OFF during start");
        currentState = OFF;
        controlStarter(false);
        controlGlowPlugs(false);
      } else if (!keyStartHeld || keyPosition < 3) {  // Key released from START position
        Serial.println("Start key released - assuming engine started, returning to ON");
        currentState = RUNNING;
        controlStarter(false);
        // Note: Let glow plugs continue their natural cycle - don't turn them off here
      } else if (millis() - ignitionStartTime >= IGNITION_TIMEOUT) {
        // Start timeout - stop cranking but stay in START until key is released
        Serial.println("Engine start timeout - stopping cranking (release key)");
        controlStarter(false);
        // Don't change state until key is released
      }
      break;
      
    case RUNNING:
      // Engine running - key should be in ON position
      if (keyPosition == 0) {  // Key turned to OFF
        Serial.println("KEY OFF - Engine shutdown (should stop engine manually first)");
        currentState = OFF;
      } else if (keyPosition >= 3 || keyStartHeld) {  // Key turned to START again
        Serial.println("HOT RESTART - Engine already running, brief cranking");
        currentState = START;
        ignitionStartTime = millis();
        startHoldTime = millis();
        controlStarter(true);
      }
      // Continue monitoring engine parameters
      break;
      
    case LOW_OIL_PRESSURE:
    case HIGH_TEMPERATURE:
      // Alert states - handle key positions but show warnings
      if (keyPosition == 0) {
        currentState = OFF;
      } else if (keyPosition >= 3 || keyStartHeld) {
        Serial.println("FORCED START during alert condition");
        currentState = START;
        ignitionStartTime = millis();
        startHoldTime = millis();
        controlStarter(true);
      }
      break;
      
    case ERROR:
      // Error state - only respond to key OFF or emergency override
      if (keyPosition == 0) {
        currentState = OFF;
      } else if (keyPosition >= 3 || keyStartHeld) {
        Serial.println("OVERRIDE START from error state");
        currentState = START;
        ignitionStartTime = millis();
        startHoldTime = millis();
        controlStarter(true);
      }
      break;
  }

  // Update tracking variables
  lastState = currentState;
  lastKeyPosition = keyPosition;
}
