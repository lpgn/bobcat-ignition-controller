/*
 * System State Management Header for Bobcat Ignition Controller
 * Contains the main state machine logic
 */

#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Arduino.h>

// Centralized system state structure
typedef struct {
    // Key and system state
    int keyPosition;              // 0=OFF, 1=ON, 2=GLOW_PLUG, 3=START
    bool keyStartHeld;            // True while START position is held
    unsigned long startHoldTime;  // When START position was first engaged
    
    // System state
    int currentState;             // SystemState enum value
    unsigned long glowPlugStartTime;
    unsigned long ignitionStartTime;
    unsigned long shutdownStartTime;
    bool shutdownInProgress;

    // Button and relay states
    bool emergencyStopPressed;
    bool lightsTogglePressed;
    bool workLightsOn;

    // Sensor values (add more as needed)
    float engineTemp;
    float oilPressure;
    float batteryVoltage;
    float fuelLevel;

    // Power management
    unsigned long lastActivityTime;    // Last time there was user activity
    bool sleepModeEnabled;            // Whether sleep mode is enabled
    bool wakeUpPending;               // Wake up from sleep pending
    unsigned long sleepTimer;         // Timer for sleep timeout

    // Add other state variables as needed
} SystemState_t;

// Global system state instance (defined in system_state.cpp)
extern SystemState_t g_systemState;

// State machine functions
void runIgnitionSequence();

#endif // SYSTEM_STATE_H
