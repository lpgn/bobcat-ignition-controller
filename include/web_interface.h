/*
 * Web Interface Header for Bobcat Ignition Controller
 * Contains functions for web-based control interface
 */

#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>

// Web interface functions
void initWebInterface();
void handleWebRequests();
String getBobcatStatus();

#endif // WEB_INTERFACE_H
