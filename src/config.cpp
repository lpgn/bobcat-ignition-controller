/*
 * Configuration Implementation for Bobcat Ignition Controller
 * Defines all constants and global variables
 */

#include "config.h"

// Pin definitions
const int GLOW_PLUG_RELAY_PIN = 2;    // GPIO2 for glow plug relay control
const int IGNITION_RELAY_PIN = 4;     // GPIO4 for main ignition relay

// Safety and monitoring pins
const int ENGINE_TEMP_PIN = 36;       // ADC1_CH0
const int OIL_PRESSURE_PIN = 39;      // ADC1_CH3
const int BATTERY_VOLTAGE_PIN = 34; // ADC1_CH6

// Timing constants
const unsigned long GLOW_PLUG_DURATION = 5000; // 5 seconds
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
