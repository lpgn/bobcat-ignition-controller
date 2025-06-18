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

void setup() {
  Serial.begin(115200);
  Serial.println("Bobcat Ignition Controller Starting...");
  
  initializePins();
  currentState = IDLE;
  
  Serial.println("System initialized - Ready for operation");
  Serial.println("Press START button to begin ignition sequence");
}

void loop() {
  // Fly-by-wire control - no physical buttons, only web interface
  updateSystem();
  checkSafetyInputs();
  
  // No delay needed - ESP32 handles timing efficiently with millis()
}
