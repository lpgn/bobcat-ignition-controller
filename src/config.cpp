/*
 * Configuration Implementation for Bobcat Ignition Controller
 * Defines all constants and global variables
 */

#include "config.h"

// Pin definitions
const int GLOW_PLUG_RELAY_PIN = 2;    // GPIO2 for glow plug relay control
const int IGNITION_RELAY_PIN = 4;     // GPIO4 for main ignition relay

// Safety and monitoring pins
const int ENGINE_TEMP_PIN = 34;       // GPIO34 (ADC) for engine temperature sensor
const int OIL_PRESSURE_PIN = 35;      // GPIO35 (ADC) for oil pressure sensor
const int BATTERY_VOLTAGE_PIN = 32;   // GPIO32 (ADC) for battery voltage monitoring

// Timing constants
const unsigned long GLOW_PLUG_DURATION = 20000;  // 20 seconds in milliseconds
const unsigned long IGNITION_TIMEOUT = 5000;     // 5 seconds max cranking time

// Global variables
SystemState currentState = IDLE;
unsigned long glowPlugStartTime = 0;
unsigned long ignitionStartTime = 0;
bool startButtonPressed = false;
bool stopButtonPressed = false;

// Non-blocking timing variables
unsigned long shutdownStartTime = 0;
bool shutdownInProgress = false;
