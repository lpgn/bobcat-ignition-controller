/*
 * Hardware Control Implementation for Bobcat Ignition Controller
 * Implements all hardware control functions
 */

#include "hardware.h"
#include "config.h"

void initializePins() {
  // Initialize output pins for relays
  pinMode(GLOW_PLUGS_PIN, OUTPUT);
  pinMode(IGNITION_PIN, OUTPUT);
  pinMode(STARTER_PIN, OUTPUT);

  // ADC pins for sensors (no pinMode needed for ADC)

  // Initialize all outputs to safe state
  digitalWrite(GLOW_PLUGS_PIN, LOW);
  digitalWrite(IGNITION_PIN, LOW);
}

void controlGlowPlugs(bool enable) {
  digitalWrite(GLOW_PLUGS_PIN, enable ? HIGH : LOW);
}

void controlIgnition(bool enable) {
  digitalWrite(IGNITION_PIN, enable ? HIGH : LOW);
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
