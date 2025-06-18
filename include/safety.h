/*
 * Safety Monitoring Header for Bobcat Ignition Controller
 * Contains functions for safety checks and error handling
 */

#ifndef SAFETY_H
#define SAFETY_H

#include <Arduino.h>

// Safety monitoring functions
void checkSafetyInputs();
void handleError(const char* errorMessage);

#endif // SAFETY_H
