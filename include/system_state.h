/*
 * System State Management Header for Bobcat Ignition Controller
 * Main state machine + cross-task flags (handlers set, loop() acts).
 */

#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Arduino.h>

typedef struct {
    // Key and system state
    int keyPosition;              // 0=OFF, 1=ON, 2=GLOW, 3=START
    bool keyStartHeld;            // true while START held
    unsigned long startHoldTime;

    // System state
    int currentState;             // SystemState enum
    unsigned long glowPlugStartTime;
    unsigned long ignitionStartTime;
    unsigned long shutdownStartTime;
    bool shutdownInProgress;

    // RUNNING detection while cranking
    unsigned long oilOkSince;     // when oil pressure first went OK (0 = not OK)

    // Buttons / relay flags
    bool emergencyStopPressed;
    bool lightsTogglePressed;
    bool workLightsOn;

    // Sensor values (cached)
    float engineTemp;
    float oilPressure;
    float batteryVoltage;
    float fuelLevel;

    // Power management
    unsigned long lastActivityTime;
    bool sleepModeEnabled;
    bool wakeUpPending;
    unsigned long sleepTimer;

    // Cross-task flags: web handlers (async_tcp) set these; loop() performs the
    // actual flash writes / relay actuation / sleep.
    volatile bool configDirty;          // settings changed in RAM -> persist
    volatile bool reloadCalibration;    // runtime_* must be refreshed
    volatile bool sleepRequested;       // deep-sleep requested from handler
    volatile bool factoryResetRequested;
    volatile bool overrideRequested;    // deliberate override crank (POST only)
    volatile bool wifiReconnect;        // station creds changed -> loop() re-begins STA
    volatile bool maintenanceMode;      // TEST/MAINTENANCE: bypass interlocks + battery for cranking
    unsigned long maintenanceStart;     // millis() when enabled (auto-expires after 30 min)

    // Engine-hours accumulator (ms of running time not yet folded into NVS)
    unsigned long engineHoursAccumMs;
} SystemState_t;

extern SystemState_t g_systemState;

// State machine
void runIgnitionSequence();
void updateEngineHours(unsigned long now);

// Helpers
const char* systemStateToString(int state);  // legacy long names
const char* apiStateName(int state);          // /api contract names
int apiStateSeq(int state);                    // seq 0..4

#endif // SYSTEM_STATE_H
