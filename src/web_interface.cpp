/*
 * Web Interface Implementation for Bobcat Ignition Controller
 * Provides web-based control interface (example/template)
 */

#include "web_interface.h"
#include "config.h"
#include "hardware.h"
#include "system_state.h"
#include "safety.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// WiFi credentials for the Access Point
const char* ssid = "Bobcat-Control";
const char* password = "bobcat123";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Helper function to convert SystemState enum to a string for the web interface
const char* systemStateToString(SystemState state) {
    switch (state) {
        case IDLE: return "IDLE";
        case POWER_ON: return "POWER ON";
        case GLOW_PLUG_HEATING: return "GLOW PLUG HEATING";
        case READY_TO_START: return "READY TO START";
        case STARTING: return "STARTING";
        case RUNNING: return "RUNNING";
        case LOW_OIL_PRESSURE: return "LOW OIL PRESSURE";
        case HIGH_TEMPERATURE: return "HIGH TEMPERATURE";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

// Main function to set up the web server
void setupWebServer() {
    // Initialize LittleFS
    if (!LittleFS.begin()) {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

    // Setup the ESP32 as a Wi-Fi Access Point
    WiFi.softAP(ssid, password);
    Serial.println("Access Point Started");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    // Define the routes for the web server

    // Serve the main HTML page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/index.html", "text/html");
    });

    // Serve the CSS file
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/style.css", "text/css");
    });

    // Serve the JavaScript file
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/script.js", "text/javascript");
    });

    // Serve the logo image
    server.on("/logo.png", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/logo.png", "image/png");
    });

    // Handle the start button press
    server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request){
        virtualStartButton();
        request->send(200, "text/plain", "Start request received");
    });

    // Handle the power on button press
    server.on("/power_on", HTTP_GET, [](AsyncWebServerRequest *request){
        virtualPowerOnButton();
        request->send(200, "text/plain", "Power on request received");
    });

    // Handle the power off button press
    server.on("/power_off", HTTP_GET, [](AsyncWebServerRequest *request){
        virtualPowerOffButton();
        request->send(200, "text/plain", "Power off request received");
    });

    // Note: No engine stop endpoint - engine must be stopped manually with lever

    // Handle the lights toggle
    server.on("/toggle_lights", HTTP_GET, [](AsyncWebServerRequest *request){
        virtualLightsButton();
        request->send(200, "text/plain", "Lights toggled");
    });

    // Handle the override button press
    server.on("/override", HTTP_GET, [](AsyncWebServerRequest *request){
        overrideStart();
        request->send(200, "text/plain", "Override request received");
    });

    // Provide the system status as a JSON object
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
        StaticJsonDocument<256> doc;
        doc["status"] = systemStateToString(currentState);
        doc["temperature"] = readEngineTemp();
        doc["pressure"] = readOilPressure();
        doc["battery"] = readBatteryVoltage();

        String jsonResponse;
        serializeJson(doc, jsonResponse);
        request->send(200, "application/json", jsonResponse);
    });

    // Start the server
    server.begin();
    Serial.println("Web server started");
}
