/*
 * Web Interface Header for Bobcat Ignition Controller
 * Contains functions for web-based control interface
 */

#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <ESPAsyncWebServer.h>
#include "system_state.h"  // For g_systemState access

// Function to initialize and configure the web server
void setupWebServer();

// Virtual button functions for backward compatibility
void virtualStartButton();
void virtualPowerOnButton();
void virtualPowerOffButton();
void virtualLightsButton();
void overrideStart();
void handleSetSetting(AsyncWebServerRequest *request);

/**
 * @brief Handles requests for system status.
 * 
 * @param request The HTTP request.
 */
void handleStatus(AsyncWebServerRequest *request);

#endif // WEB_INTERFACE_H
