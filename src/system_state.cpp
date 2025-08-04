/*
 * System State Management Implementation for Bobcat Ignition Controller
 * Implements the main state machine logic
 */


#include "system_state.h"
#include "config.h"
#include "hardware.h"
#include "safety.h"

// Global system state instance
SystemState_t g_systemState = {};

/*
 * System State Management Implementation for Bobcat Ignition Controller
 * Implements proper ignition key sequence: OFF -> ON -> GLOW_PLUG -> START (momentary)
 */

#include "system_state.h"
#include "config.h"
#include "hardware.h"
#include "safety.h"

void runIgnitionSequence() {
  static int lastState = -1; // Track state changes
  static int lastKeyPosition = -1; // Track key position changes
  
  // Only execute state logic when state or key position changes
  bool stateChanged = (g_systemState.currentState != lastState);
  bool keyChanged = (g_systemState.keyPosition != lastKeyPosition);
  
  // Force immediate OFF transition when key is turned to OFF position
  if (g_systemState.keyPosition == 0 && g_systemState.currentState != OFF) {
    Serial.println("FORCE OFF - Key turned to OFF position");
    g_systemState.currentState = OFF;
    stateChanged = true;
    controlMainPower(false);
    controlGlowPlugs(false);
    controlStarter(false);
    controlLights(false);
    g_systemState.workLightsOn = false;
  }
  
  // Handle emergency stop (always process immediately)
  if (g_systemState.emergencyStopPressed) {
    Serial.println("EMERGENCY STOP activated - stopping engine processes only");
    // Stop engine processes but keep main power for lights
    controlGlowPlugs(false);
    controlStarter(false);
    // Keep main power ON so lights can still work
    // Keep work lights in their current state
    g_systemState.keyPosition = 1;  // Force key to ON position (not OFF) to maintain main power
    if (g_systemState.currentState != ON) {
      g_systemState.currentState = ON;  // Return to ON state, not OFF
      stateChanged = true;
    }
    g_systemState.emergencyStopPressed = false;
  }

  // Handle lights toggle (independent of engine state - always process)
  if (g_systemState.lightsTogglePressed) {
    g_systemState.workLightsOn = !g_systemState.workLightsOn;
    controlLights(g_systemState.workLightsOn);
    Serial.println(g_systemState.workLightsOn ? "Work lights ON" : "Work lights OFF");
    g_systemState.lightsTogglePressed = false;
  }
  
  if (!stateChanged && !keyChanged) {
    // Handle time-based logic for glow plugs (works in any state)
    if (g_systemState.currentState == GLOW_PLUG || g_systemState.currentState == START || g_systemState.currentState == RUNNING) {
      if (millis() - g_systemState.glowPlugStartTime >= GLOW_PLUG_DURATION) {
        // Glow plug heating complete - turn off glow plugs
        controlGlowPlugs(false);
        if (g_systemState.currentState == GLOW_PLUG) {
          Serial.println("Glow plug heating complete - ready for start");
        }
      }
    }
    return; // No state change, no need to execute switch statement
  }
  
  // Update tracking variables
  lastState = g_systemState.currentState;
  lastKeyPosition = g_systemState.keyPosition;
  
  // Handle key position changes and automatic state transitions
  switch (g_systemState.currentState) {
    case OFF:
      // Key OFF - Turn off ALL power including lights (master power off)
      if (stateChanged) {
        controlMainPower(false);
        controlGlowPlugs(false);
        controlStarter(false);
        controlLights(false);  // OFF turns off everything including lights
        g_systemState.workLightsOn = false;  // Reset work lights state
        Serial.println("KEY OFF - System shutdown");
      }
      
      if (g_systemState.keyPosition >= 1) {  // Key turned to ON position or beyond
        Serial.println("KEY ON - System energized");
        g_systemState.currentState = ON;
        controlMainPower(true);
      }
      break;
      
    case ON:
      // Key ON - Basic electrical systems active
      
      // Check if glow plugs should be turned off (timer expired while in ON)
      if (g_systemState.glowPlugStartTime > 0 && millis() - g_systemState.glowPlugStartTime >= GLOW_PLUG_DURATION) {
        controlGlowPlugs(false);
        Serial.println("Glow plug timer expired while in ON position");
      }
      
      if (g_systemState.keyPosition == 0) {  // Key turned back to OFF
        Serial.println("KEY OFF - System shutdown");
        g_systemState.currentState = OFF;
      } else if (g_systemState.keyPosition == 2) {  // Key turned to GLOW position
        Serial.println("GLOW PLUG position - Starting/resuming glow plug heating");
        g_systemState.currentState = GLOW_PLUG;
        // Only start timer if not already running (allow resuming)
        if (g_systemState.glowPlugStartTime == 0 || millis() - g_systemState.glowPlugStartTime >= GLOW_PLUG_DURATION) {
          g_systemState.glowPlugStartTime = millis();  // Start new cycle
          Serial.println("Starting new glow plug cycle (20 seconds)");
        } else {
          Serial.println("Resuming existing glow plug cycle");
        }
        controlGlowPlugs(true);
      } else if (g_systemState.keyPosition >= 3 || g_systemState.keyStartHeld) {  // Direct START from ON (auto-glow)
        Serial.println("Direct START - Auto-activating glow plugs and cranking");
        g_systemState.currentState = START;
        g_systemState.glowPlugStartTime = millis();  // Start glow plug timer
        g_systemState.ignitionStartTime = millis();
        g_systemState.startHoldTime = millis();
        controlGlowPlugs(true);  // Auto-activate glow plugs for direct start
        controlStarter(true);    // Start cranking
      }
      break;
      
    case GLOW_PLUG:
      // Glow plug heating phase
      if (g_systemState.keyPosition == 0) {  // Key turned to OFF
        Serial.println("KEY OFF during glow plug heating");
        g_systemState.currentState = OFF;
        controlGlowPlugs(false);
      } else if (g_systemState.keyPosition == 1) {  // Key returned to ON
        Serial.println("Key returned to ON - pausing glow plug heating");
        g_systemState.currentState = ON;
        // Note: Don't turn off glow plugs immediately - let them continue heating
        // This allows user to return to GLOW position without losing progress
      } else if (g_systemState.keyPosition >= 3 || g_systemState.keyStartHeld) {  // Key turned to START
        Serial.println("START position - Engine cranking");
        g_systemState.currentState = START;
        g_systemState.ignitionStartTime = millis();
        g_systemState.startHoldTime = millis();
        // Keep glow plugs ON during start for full duration - some engines need this
        controlStarter(true);     // Start cranking
      }
      
      // Check if glow plug heating is complete
      if (millis() - g_systemState.glowPlugStartTime >= GLOW_PLUG_DURATION) {
        // Glow plug heating complete - turn off glow plugs and return key to ON
        Serial.println("Glow plug heating complete - automatically returning to ON position");
        controlGlowPlugs(false);
        g_systemState.currentState = ON;
        g_systemState.keyPosition = 1;  // Automatically return key to ON position
      } else {
        // Show countdown every 2 seconds during heating (only while heating)
        static unsigned long lastCountdown = 0;
        if (millis() - lastCountdown >= 2000) {
          unsigned long elapsed = millis() - g_systemState.glowPlugStartTime;
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
      if (millis() - g_systemState.glowPlugStartTime >= GLOW_PLUG_DURATION) {
        controlGlowPlugs(false);
      }
      
      if (g_systemState.keyPosition == 0) {  // Key turned to OFF
        Serial.println("KEY OFF during start");
        g_systemState.currentState = OFF;
        controlStarter(false);
        controlGlowPlugs(false);
      } else if (!g_systemState.keyStartHeld || g_systemState.keyPosition < 3) {  // Key released from START position
        Serial.println("Start key released - returning to ON position");
        g_systemState.currentState = ON;  // Return to ON, not GLOW_PLUG
        g_systemState.keyPosition = 1;    // Set key position to ON
        controlStarter(false);
        // Turn off glow plugs when returning to ON after start attempt
        controlGlowPlugs(false);
      } else if (millis() - g_systemState.ignitionStartTime >= IGNITION_TIMEOUT) {
        // Start timeout - stop cranking but stay in START until key is released
        Serial.println("Engine start timeout - stopping cranking (release key)");
        controlStarter(false);
        // Don't change state until key is released
      }
      break;
      
    case RUNNING:
      // Engine running - key should be in ON position
      if (g_systemState.keyPosition == 0) {  // Key turned to OFF
        Serial.println("KEY OFF - Engine shutdown (should stop engine manually first)");
        g_systemState.currentState = OFF;
      } else if (g_systemState.keyPosition >= 3 || g_systemState.keyStartHeld) {  // Key turned to START again
        Serial.println("HOT RESTART - Engine already running, brief cranking");
        g_systemState.currentState = START;
        g_systemState.ignitionStartTime = millis();
        g_systemState.startHoldTime = millis();
        controlStarter(true);
      }
      // Continue monitoring engine parameters
      break;
      
    case LOW_OIL_PRESSURE:
    case HIGH_TEMPERATURE:
      // Alert states - handle key positions but show warnings
      if (g_systemState.keyPosition == 0) {
        g_systemState.currentState = OFF;
      } else if (g_systemState.keyPosition >= 3 || g_systemState.keyStartHeld) {
        Serial.println("FORCED START during alert condition");
        g_systemState.currentState = START;
        g_systemState.ignitionStartTime = millis();
        g_systemState.startHoldTime = millis();
        controlStarter(true);
      }
      break;
      
    case ERROR:
      // Error state - only respond to key OFF or emergency override
      if (g_systemState.keyPosition == 0) {
        g_systemState.currentState = OFF;
      } else if (g_systemState.keyPosition >= 3 || g_systemState.keyStartHeld) {
        Serial.println("OVERRIDE START from error state");
        g_systemState.currentState = START;
        g_systemState.ignitionStartTime = millis();
        g_systemState.startHoldTime = millis();
        controlStarter(true);
      }
      break;
  }

  // Update tracking variables
  lastState = g_systemState.currentState;
  lastKeyPosition = g_systemState.keyPosition;
}
