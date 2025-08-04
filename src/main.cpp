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
  g_systemState.currentState = OFF;  // Start in OFF state like a real ignition
  g_systemState.keyPosition = 0;     // Key starts in OFF position
  
  setupWebServer(); // Initialize the web server

  Serial.println("System initialized - Key is in OFF position");
  Serial.println("Turn key to ON, then GLOW PLUG, then hold START to crank engine");
}

void loop() {
  // Main ignition key sequence control
  runIgnitionSequence();
  
  // Only check engine vitals when running
  if (g_systemState.currentState == RUNNING) { 
    checkEngineVitals();
  } 
  
  // Always run safety checks except during start cranking
  if (g_systemState.currentState != START) {
    checkSafetyInputs();
  }
  
  // Small delay to prevent overwhelming the system
  delay(10);
}
