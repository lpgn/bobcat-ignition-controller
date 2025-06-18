/*
 * Hardware Control Header for Bobcat Ignition Controller
 * Contains functions for controlling physical hardware components
 */

#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>

// Hardware initialization
void initializePins();

// Virtual button functions for web interface
void virtualStartButton();     // Virtual start button for web interface
void virtualStopButton();      // Virtual stop button for web interface

// Hardware control functions
void controlGlowPlugs(bool enable);
void controlIgnition(bool enable);

#endif // HARDWARE_H
