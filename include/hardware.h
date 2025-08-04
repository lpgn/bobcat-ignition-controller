/*
 * Hardware Control Header for Bobcat Ignition Controller
 * Contains functions for controlling physical hardware components
 * Simplified Diesel Engine Interface
 */

#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include "system_state.h"  // For g_systemState access

// ============================================================================
// HARDWARE INITIALIZATION
// ============================================================================
void initializePins();

// ============================================================================
// RELAY CONTROL FUNCTIONS - Diesel Engine Control
// ============================================================================
void controlMainPower(bool enable);
void controlGlowPlugs(bool enable);      // Glow plug relay control
void controlStarter(bool enable);        // Starter solenoid relay
void controlLights(bool enable);         // Both front and back lights relay

// ============================================================================
// VIRTUAL BUTTON FUNCTIONS - Web Interface Control
// ============================================================================
void virtualPowerOnButton();
void virtualPowerOffButton();
void virtualStartButton();     // Virtual start button for web interface
void virtualLightsButton();    // Combined front and back lights toggle
// Note: No virtualStopButton - engine must be stopped manually with lever

// ============================================================================
// ANALOG SENSOR READING FUNCTIONS
// ============================================================================
float readEngineTemp();        // Coolant temperature (°C)
float readOilPressure();       // Oil pressure (kPa)
float readBatteryVoltage();    // Battery voltage (V)
float readFuelLevel();         // Fuel level (%)

// ============================================================================
// DIGITAL INPUT READING FUNCTIONS
// ============================================================================
bool readAlternatorCharge();     // Alternator charging status
bool readEngineRunFeedback();   // Engine running feedback
// SAFETY CHECK FUNCTIONS
// ============================================================================
bool checkEngineSafetyConditions();  // Check all safety interlocks
bool isEngineRunning();              // Detect if engine is actually running
void performSafetyShutdown();        // Safe emergency shutdown sequence
#endif // HARDWARE_H
