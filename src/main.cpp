/*
 * Bobcat Ignition Controller - Main File
 * ESP32-based system to control Bobcat ignition sequence
 * 
 * Features:
 * - Glow plug preheating (20 seconds)
 * - Main ignition control
 * - Safety interlocks
 * - Status monitoring
 * 
 * Author: Generated for Bobcat Controller Project
 * Date: June 2025
 */

#include <Arduino.h>
#include "config.h"
#include "hardware.h"
#include "safety.h"
#include "system_state.h"
#include "web_interface.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Bobcat Ignition Controller Starting...");
  
  initializePins();
  currentState = IDLE;
  
  setupWebServer(); // Initialize the web server

  Serial.println("System initialized - Ready for operation");
  Serial.println("Press START button to begin ignition sequence");
}

void loop() {
  // Fly-by-wire control - no physical buttons, only web interface
  runIgnitionSequence();
  if (currentState == RUNNING) { 
    checkEngineVitals();
  } else if (currentState != STARTING) { // Don't run safety checks when overriding
    checkSafetyInputs();
  }
  
  // No delay needed - ESP32 handles timing efficiently with millis()
}
