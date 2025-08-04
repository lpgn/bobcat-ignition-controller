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
        case IDLE: return "OFF";
        case POWER_ON: return "ON"; 
        case GLOW_PLUG_HEATING: return "GLOW_HEATING";
        case READY_TO_START: return "READY";
        case STARTING: return "STARTING";
        case RUNNING: return "RUNNING";
        case LOW_OIL_PRESSURE: return "LOW_OIL_PRESSURE";
        case HIGH_TEMPERATURE: return "HIGH_TEMPERATURE";
        case ERROR: return "EMERGENCY_STOP";
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

    // Captive portal - redirect all requests to our main page
    server.onNotFound([](AsyncWebServerRequest *request){
        Serial.print("Captive portal redirect for: ");
        Serial.println(request->url());
        request->redirect("http://192.168.4.1/");
    });

    // Handle captive portal detection requests
    server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request){
        request->redirect("http://192.168.4.1/");
    });
    
    server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request){
        request->redirect("http://192.168.4.1/");
    });
    
    server.on("/check_network_status.txt", HTTP_GET, [](AsyncWebServerRequest *request){
        request->redirect("http://192.168.4.1/");
    });
    
    server.on("/library/test/success.html", HTTP_GET, [](AsyncWebServerRequest *request){
        request->redirect("http://192.168.4.1/");
    });
    
    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request){
        request->redirect("http://192.168.4.1/");
    });

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

    // Handle the start button press (keep for backward compatibility)
    server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("Legacy start endpoint called");
        virtualStartButton();
        request->send(200, "text/plain", "Start request received");
    });

    // Handle the power on button press (keep for backward compatibility)
    server.on("/power_on", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("Legacy power_on endpoint called");
        virtualPowerOnButton();
        request->send(200, "text/plain", "Power on request received");
    });

    // Handle the power off button press (keep for backward compatibility)
    server.on("/power_off", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("Legacy power_off endpoint called");
        virtualPowerOffButton();
        request->send(200, "text/plain", "Power off request received");
    });

    // Note: No engine stop endpoint - engine must be stopped manually with lever

    // Handle the lights toggle (keep for backward compatibility)
    server.on("/toggle_lights", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("Legacy toggle_lights endpoint called");
        virtualLightsButton();
        request->send(200, "text/plain", "Lights toggled");
    });

    // Handle the override button press (keep for backward compatibility)
    server.on("/override", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("Legacy override endpoint called");
        overrideStart();
        request->send(200, "text/plain", "Override request received");
    });

    // Handle unified control endpoint for the new dashboard
    server.on("/control", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            // Parse JSON command
            StaticJsonDocument<256> doc;
            DeserializationError error = deserializeJson(doc, (char*)data, len);
            
            if (error) {
                Serial.println("Failed to parse JSON command");
                request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
                return;
            }
            
            String action = doc["action"];
            Serial.print("Web command received: ");
            Serial.println(action);
            
            bool success = true;
            String message = "Command executed";
            
            // Execute the requested action
            if (action == "start") {
                virtualStartButton();
            } else if (action == "stop_crank") {
                // Stop cranking - turn off starter relay
                digitalWrite(STARTER_PIN, LOW);
                Serial.println("Starter relay OFF - cranking stopped");
                // Return to appropriate state based on current state
                if (currentState == STARTING) {
                    currentState = POWER_ON; // Return to power on state
                }
            } else if (action == "power_on") {
                virtualPowerOnButton();
            } else if (action == "shutdown") {
                virtualPowerOffButton();
            } else if (action == "emergency_stop") {
                // Force emergency stop state
                currentState = ERROR;
                digitalWrite(STARTER_PIN, LOW);
                digitalWrite(LIGHTS_PIN, LOW);
                digitalWrite(GLOW_PLUGS_PIN, LOW);
                digitalWrite(MAIN_POWER_PIN, LOW);
                Serial.println("EMERGENCY STOP activated via web interface");
            } else if (action == "lights") {
                virtualLightsButton();
            } else if (action == "horn") {
                // Horn action - could trigger a horn relay if available
                Serial.println("Horn activated via web interface");
            } else {
                success = false;
                message = "Unknown action: " + action;
            }
            
            // Send response
            StaticJsonDocument<256> response;
            response["success"] = success;
            response["message"] = message;
            
            String jsonResponse;
            serializeJson(response, jsonResponse);
            request->send(success ? 200 : 400, "application/json", jsonResponse);
        });

    // Provide the system status as a JSON object
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
        StaticJsonDocument<512> doc;
        doc["state"] = systemStateToString(currentState);
        doc["status"] = systemStateToString(currentState); // Keep for backward compatibility
        doc["temperature"] = readEngineTemp();
        doc["pressure"] = readOilPressure();
        doc["battery"] = readBatteryVoltage();
        doc["battery_voltage"] = readBatteryVoltage();
        doc["engine_temp"] = readEngineTemp();
        doc["oil_pressure"] = readOilPressure();
        
        // Add state flags for dashboard
        doc["lights_on"] = digitalRead(LIGHTS_PIN);
        doc["engine_fault"] = (currentState == ERROR);
        doc["low_oil_pressure"] = (currentState == LOW_OIL_PRESSURE);
        doc["high_temperature"] = (currentState == HIGH_TEMPERATURE);
        doc["low_battery"] = (readBatteryVoltage() < 11.5);
        
        // Add mock data for missing sensors
        doc["fuel_level"] = 75;
        doc["engine_hours"] = 1234;
        
        // Add glow plug countdown
        if (currentState == GLOW_PLUG_HEATING) {
            unsigned long elapsed = millis() - glowPlugStartTime;
            unsigned long remaining = (GLOW_PLUG_DURATION - elapsed) / 1000;
            doc["countdown"] = remaining;
        } else {
            doc["countdown"] = 0;
        }

        String jsonResponse;
        serializeJson(doc, jsonResponse);
        request->send(200, "application/json", jsonResponse);
    });

    // Start the server
    server.begin();
    Serial.println("Web server started");
}
