/*
 * Configuration Header for Bobcat Ignition Controller
 * Contains all pin definitions, constants, and system configuration
 */

#ifndef CONFIG_H
#define CONFIG_H

// Pin definitions
extern const int GLOW_PLUG_RELAY_PIN;
extern const int IGNITION_RELAY_PIN;

// Safety and monitoring pins
extern const int ENGINE_TEMP_PIN;
extern const int OIL_PRESSURE_PIN;
extern const int BATTERY_VOLTAGE_PIN;

// Timing constants
extern const unsigned long GLOW_PLUG_DURATION;
extern const unsigned long IGNITION_TIMEOUT;

// System states
enum SystemState {
  IDLE,
  GLOW_PLUG_HEATING,
  READY_TO_START,
  STARTING,
  RUNNING,
  SHUTDOWN,
  ERROR
};

// Global system variables
extern SystemState currentState;
extern unsigned long glowPlugStartTime;
extern unsigned long ignitionStartTime;
extern bool startButtonPressed;
extern bool stopButtonPressed;

// Non-blocking timing variables
extern unsigned long shutdownStartTime;
extern bool shutdownInProgress;

#endif // CONFIG_H
