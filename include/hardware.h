/*
 * Hardware Control Header for Bobcat Ignition Controller
 * Contains functions for controlling physical hardware components
 * Simplified Diesel Engine Interface
 */

#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>

// ============================================================================
// HARDWARE INITIALIZATION
// ============================================================================
void initializePins();

// ============================================================================
// RELAY CONTROL FUNCTIONS - Diesel Engine Control
// ============================================================================
void controlGlowPlugs(bool enable);      // Glow plug relay control
void controlIgnition(bool enable);       // Main ignition/run relay
void controlStarter(bool enable);        // Starter solenoid relay

// ============================================================================
// VIRTUAL BUTTON FUNCTIONS - Web Interface Control
// ============================================================================
void virtualStartButton();     // Virtual start button for web interface
void virtualStopButton();      // Virtual stop button for web interface

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
