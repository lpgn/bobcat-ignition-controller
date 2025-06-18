/*
 * Configuration Header for Bobcat Ignition Controller
 * Contains all pin definitions, constants, and system configuration
 */

#ifndef CONFIG_H
#define CONFIG_H

// Pin definitions
// Use "safe" GPIOs that are not pulled high on boot
const int IGNITION_PIN = 23;
const int STARTER_PIN = 22;
const int GLOW_PLUGS_PIN = 21;

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
