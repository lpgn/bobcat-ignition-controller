/*
 * Hardware Control Implementation for Bobcat Ignition Controller
 * Implements all hardware control functions
 */

#include "hardware.h"
#include "config.h"

void initializePins() {
  // Output pins only - no physical buttons
  pinMode(GLOW_PLUG_RELAY_PIN, OUTPUT);
  pinMode(IGNITION_RELAY_PIN, OUTPUT);
  
  // ADC pins for sensors (no pinMode needed for ADC)
  
  // Initialize all outputs to safe state
  digitalWrite(GLOW_PLUG_RELAY_PIN, LOW);
  digitalWrite(IGNITION_RELAY_PIN, LOW);
}

void controlGlowPlugs(bool enable) {
  digitalWrite(GLOW_PLUG_RELAY_PIN, enable ? HIGH : LOW);
  Serial.print("Glow plugs: ");
  Serial.println(enable ? "ON" : "OFF");
}

void controlIgnition(bool enable) {
  digitalWrite(IGNITION_RELAY_PIN, enable ? HIGH : LOW);
  Serial.print("Ignition/Starter: ");
  Serial.println(enable ? "ON" : "OFF");
}

// Virtual button functions for web interface - Tesla-style fly-by-wire control
void virtualStartButton() {
  if (!startButtonPressed) {
    startButtonPressed = true;
    Serial.println("Web Interface: START button pressed");
  }
}

void virtualStopButton() {
  if (!stopButtonPressed) {
    stopButtonPressed = true;
    Serial.println("Web Interface: STOP button pressed");
  }
}
