/*
 * Safety Monitoring Header for Bobcat Ignition Controller
 * Contains functions for safety checks and error handling
 */

#ifndef SAFETY_H
#define SAFETY_H

#include <Arduino.h>
#include "system_state.h"  // For g_systemState access

// Safety monitoring functions
void checkSafetyInputs();
void handleError(const char* errorMessage);
void checkEngineVitals();

// Override function
void overrideStart();

#endif // SAFETY_H
