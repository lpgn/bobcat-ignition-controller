/*
 * Web Interface Implementation for Bobcat Ignition Controller
 * Provides web-based control interface (example/template)
 */

#include "web_interface.h"
#include "config.h"
#include "hardware.h"

// This is a template - you'll need to add your preferred web server library
// Examples: ESP32WebServer, AsyncWebServer, etc.

void initWebInterface() {
  Serial.println("Initializing Web Interface...");
  // Initialize your web server here
  // Example:
  // server.begin();
  // WiFi.begin(ssid, password);
}

void handleWebRequests() {
  // Handle incoming web requests
  // This function should be called from main loop
  
  // Example of how to handle button presses from web interface:
  /*
  server.on("/start", HTTP_GET, []() {
    virtualStartButton();  // Call the virtual start button function
    server.send(200, "text/plain", "Start button pressed");
  });
  
  server.on("/stop", HTTP_GET, []() {
    virtualStopButton();   // Call the virtual stop button function
    server.send(200, "text/plain", "Stop button pressed");
  });
  
  server.on("/status", HTTP_GET, []() {
    String status = getSystemStatus();
    server.send(200, "application/json", status);
  });
  
  server.handleClient();
  */
}

String getSystemStatus() {
  // Return system status as JSON for web interface
  String status = "{";
  status += "\"state\":\"";
  
  switch (currentState) {
    case IDLE:
      status += "IDLE";
      break;
    case GLOW_PLUG_HEATING:
      status += "GLOW_PLUG_HEATING";
      break;
    case READY_TO_START:
      status += "READY_TO_START";
      break;
    case STARTING:
      status += "STARTING";
      break;
    case RUNNING:
      status += "RUNNING";
      break;
    case SHUTDOWN:
      status += "SHUTDOWN";
      break;
    case ERROR:
      status += "ERROR";
      break;
  }
  
  status += "\",";
  status += "\"glowPlugTime\":" + String(millis() - glowPlugStartTime) + ",";
  status += "\"ignitionTime\":" + String(millis() - ignitionStartTime);
  status += "}";
  
  return status;
}
