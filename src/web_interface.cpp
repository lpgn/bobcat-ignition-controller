/*
 * Web Interface Implementation for Bobcat Ignition Controller
 * Provides web-based control interface (example/template)
 */

#include "web_interface.h"
#include "config.h"
#include "hardware.h"
#include "system_state.h"
#include "safety.h"
#include "settings.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>
#include <Preferences.h>

// WiFi credentials for the Access Point
const char* ssid = "Bobcat-Control";
const char* password = "bobcat123";

// WiFi credentials for connecting to home network
const char* home_ssid = "fabfarm";
const char* home_password = "imakestuff";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void handleGetSettings(AsyncWebServerRequest *request) {
    StaticJsonDocument<1024> doc;
    
    const BobcatSettings& settings = g_settingsManager.getSettings();
    
    // Engine Parameters (convert milliseconds to seconds for UI)
    doc["glowDuration"] = settings.glowPlugDuration / 1000;
    doc["crankingTimeout"] = settings.crankingTimeout / 1000;
    doc["cooldownDuration"] = settings.cooldownDuration / 1000;
    
    // Alarm Thresholds
    doc["maxTemp"] = settings.maxCoolantTemp;
    doc["minOilPressure"] = settings.minOilPressure;
    doc["minHydPressure"] = settings.minHydPressure;
    doc["minVoltage"] = settings.minBatteryVoltage;
    doc["maxVoltage"] = settings.maxBatteryVoltage;
    
    // WiFi Configuration (don't send password for security)
    doc["wifiSSID"] = settings.wifiSSID;
    
    // Sensor Calibration (only threshold is user-configurable)
    doc["fuelLevelLowThreshold"] = settings.fuelLevelLowThreshold;
    doc["pressureScale"] = settings.pressureScale;
    doc["hydPressureScale"] = settings.hydPressureScale;
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    request->send(200, "application/json", jsonResponse);
}

void onSetSettingBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    DynamicJsonDocument doc(1024);
    if (deserializeJson(doc, (const char*)data)) {
        request->send(400, "text/plain", "Invalid JSON");
        return;
    }

    const char* key = doc["key"];
    const char* value = doc["value"];

    if (key && value) {
        bool success = false;
        String message = "Setting updated";
        
        // Convert value to appropriate type and update settings
        if (strcmp(key, "glowDuration") == 0) {
            uint32_t seconds = atoi(value);
            const BobcatSettings& current = g_settingsManager.getSettings();
            success = g_settingsManager.updateEngineSettings(seconds * 1000, current.crankingTimeout, current.cooldownDuration);
        } else if (strcmp(key, "crankingTimeout") == 0) {
            uint32_t seconds = atoi(value);
            const BobcatSettings& current = g_settingsManager.getSettings();
            success = g_settingsManager.updateEngineSettings(current.glowPlugDuration, seconds * 1000, current.cooldownDuration);
        } else if (strcmp(key, "cooldownDuration") == 0) {
            uint32_t seconds = atoi(value);
            const BobcatSettings& current = g_settingsManager.getSettings();
            success = g_settingsManager.updateEngineSettings(current.glowPlugDuration, current.crankingTimeout, seconds * 1000);
        } else if (strcmp(key, "maxTemp") == 0) {
            int16_t temp = atoi(value);
            const BobcatSettings& current = g_settingsManager.getSettings();
            success = g_settingsManager.updateAlarmThresholds(temp, current.minOilPressure, current.minBatteryVoltage, current.maxBatteryVoltage);
        } else if (strcmp(key, "minOilPressure") == 0) {
            int16_t pressure = atoi(value);
            const BobcatSettings& current = g_settingsManager.getSettings();
            success = g_settingsManager.updateAlarmThresholds(current.maxCoolantTemp, pressure, current.minBatteryVoltage, current.maxBatteryVoltage);
        } else if (strcmp(key, "minHydPressure") == 0) {
            int16_t pressure = atoi(value);
            success = g_settingsManager.updateHydraulicThreshold(pressure);
        } else if (strcmp(key, "minVoltage") == 0) {
            float voltage = atof(value);
            const BobcatSettings& current = g_settingsManager.getSettings();
            success = g_settingsManager.updateAlarmThresholds(current.maxCoolantTemp, current.minOilPressure, voltage, current.maxBatteryVoltage);
        } else if (strcmp(key, "maxVoltage") == 0) {
            float voltage = atof(value);
            const BobcatSettings& current = g_settingsManager.getSettings();
            success = g_settingsManager.updateAlarmThresholds(current.maxCoolantTemp, current.minOilPressure, current.minBatteryVoltage, voltage);
        } else if (strcmp(key, "wifiSSID") == 0) {
            const BobcatSettings& current = g_settingsManager.getSettings();
            success = g_settingsManager.updateWifiSettings(value, current.wifiPassword);
        } else if (strcmp(key, "wifiPassword") == 0) {
            const BobcatSettings& current = g_settingsManager.getSettings();
            success = g_settingsManager.updateWifiSettings(current.wifiSSID, value);
        } else if (strcmp(key, "fuelLevelLowThreshold") == 0) {
            uint8_t threshold = atoi(value);
            const BobcatSettings& current = g_settingsManager.getSettings();
            success = g_settingsManager.updateSensorCalibration(current.tempSensorOffset, current.pressureScale, current.fuelLevelEmpty, current.fuelLevelFull, threshold);
        } else {
            message = "Unknown setting key: " + String(key);
        }
        
        if (success) {
            g_settingsManager.saveSettings();
            request->send(200, "text/plain", message);
        } else {
            request->send(400, "text/plain", "Failed to update setting: " + String(key));
        }
    } else {
        request->send(400, "text/plain", "Missing key or value in JSON");
    }
}

void handleStatus(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument json(1024);

    json["systemState"] = systemStateToString(g_systemState.currentState);

    if (WiFi.status() == WL_CONNECTED) {
        json["homeNetworkStatus"] = WiFi.localIP().toString();
    } else {
        json["homeNetworkStatus"] = "Not Connected";
    }
    json["apStatus"] = "Active at " + WiFi.softAPIP().toString();

    serializeJson(json, *response);
    request->send(response);
}

// Main function to set up the web server
void setupWebServer() {
    // Initialize LittleFS
    if (!LittleFS.begin()) {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

    // Set WiFi to dual mode (Station + Access Point)
    WiFi.mode(WIFI_AP_STA);
    
    // Try to connect to home network first
    Serial.println("Connecting to home network...");
    WiFi.begin(home_ssid, home_password);
    
    // Wait up to 10 seconds for connection
    int wifi_timeout = 0;
    while (WiFi.status() != WL_CONNECTED && wifi_timeout < 20) {
        delay(500);
        Serial.print(".");
        wifi_timeout++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("Connected to home network!");
        Serial.print("Home network IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("SSID: ");
        Serial.println(home_ssid);
    } else {
        Serial.println();
        Serial.println("Failed to connect to home network - continuing with AP only");
    }
    
    // Always start the Access Point (standalone mode)
    WiFi.softAP(ssid, password);
    Serial.println("Access Point Started");
    Serial.print("AP SSID: ");
    Serial.println(ssid);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    
    // Print connection summary
    Serial.println("=== WiFi Status ===");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("Home Network: CONNECTED (");
        Serial.print(WiFi.localIP());
        Serial.println(")");
    } else {
        Serial.println("Home Network: DISCONNECTED");
    }
    Serial.print("Access Point: ACTIVE (");
    Serial.print(WiFi.softAPIP());
    Serial.println(")");
    Serial.println("==================");

    // Define the routes for the web server

    // Captive portal - redirect all requests to our main page
    server.onNotFound([](AsyncWebServerRequest *request){
        Serial.print("Captive portal redirect for: ");
        Serial.println(request->url());
        // Always redirect to AP IP for simplicity
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
            
            // Update activity timer for any web command
            updateActivityTimer();
            
            bool success = true;
            String message = "Command executed";
            
            // Execute the requested action
            if (action == "key_position") {
                int requestedPosition = doc["position"];
                Serial.print("Key position command: ");
                Serial.print(g_systemState.keyPosition);
                Serial.print(" -> ");
                Serial.println(requestedPosition);
                
                // Simply accept the command - let the state machine handle all logic
                if (requestedPosition >= 0 && requestedPosition <= 3) {
                    g_systemState.keyPosition = requestedPosition;
                    message = "Key position command accepted: " + String(requestedPosition);
                    Serial.print("Key position set to: ");
                    Serial.println(requestedPosition);
                } else {
                    success = false;
                    message = "Invalid key position: " + String(requestedPosition);
                }
            } else if (action == "key_start_hold") {
                bool held = doc["held"];
                Serial.print("Start key ");
                Serial.println(held ? "held" : "released");
                
                g_systemState.keyStartHeld = held;
                if (held) {
                    g_systemState.startHoldTime = millis();
                    g_systemState.keyPosition = 3; // Move to START position
                } else {
                    g_systemState.keyPosition = 2; // Return to GLOW position when released
                }
                message = held ? "Start key held" : "Start key released";
            } else if (action == "emergency_stop") {
                // Force emergency stop state
                g_systemState.emergencyStopPressed = true;
                Serial.println("EMERGENCY STOP activated via web interface");
                message = "Emergency stop activated";
            } else if (action == "lights") {
                g_systemState.lightsTogglePressed = true;
                message = "Lights toggled";
            } else if (action == "start") {
                // Legacy support - keep for backward compatibility
                virtualStartButton();
                message = "Legacy start command";
            } else if (action == "power_on") {
                // Legacy support
                virtualPowerOnButton();
                message = "Legacy power on command";
            } else if (action == "shutdown") {
                // Legacy support
                virtualPowerOffButton();
                message = "Legacy shutdown command";
            } else if (action == "sleep_now") {
                // Immediate sleep command
                if (checkSleepConditions(true)) { // Pass true for manual sleep
                    message = "Entering sleep mode immediately";
                    Serial.println("Manual sleep command received");
                    // Send response first, then sleep
                    StaticJsonDocument<256> response;
                    response["success"] = true;
                    response["message"] = message;
                    String jsonResponse;
                    serializeJson(response, jsonResponse);
                    request->send(200, "application/json", jsonResponse);
                    delay(100); // Give time for response to send
                    enterDeepSleep();
                    return; // Won't reach here
                } else {
                    success = false;
                    message = "Cannot sleep - unsafe conditions (engine running or system active)";
                }
            } else if (action == "toggle_sleep_mode") {
                // Toggle sleep mode enable/disable
                g_systemState.sleepModeEnabled = !g_systemState.sleepModeEnabled;
                message = g_systemState.sleepModeEnabled ? "Sleep mode enabled" : "Sleep mode disabled";
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
        doc["state"] = systemStateToString(g_systemState.currentState);
        doc["status"] = systemStateToString(g_systemState.currentState); // Keep for backward compatibility
        doc["temperature"] = readEngineTemp();
        doc["pressure"] = readOilPressure();
        doc["battery"] = readBatteryVoltage();
        doc["battery_voltage"] = readBatteryVoltage();
        doc["engine_temp"] = readEngineTemp();
        doc["oil_pressure"] = readOilPressure();
        doc["hyd_pressure"] = readHydraulicPressure();
        
        // Add state flags for dashboard
        doc["lights_on"] = digitalRead(LIGHTS_PIN);
        doc["main_power_on"] = digitalRead(MAIN_POWER_PIN);
        doc["glow_plugs_on"] = digitalRead(GLOW_PLUGS_PIN);
        doc["starter_on"] = digitalRead(STARTER_PIN);
        doc["engine_fault"] = (g_systemState.currentState == ERROR);
        doc["low_oil_pressure"] = (g_systemState.currentState == LOW_OIL_PRESSURE);
        doc["high_temperature"] = (g_systemState.currentState == HIGH_TEMPERATURE);
        doc["low_battery"] = (readBatteryVoltage() < 11.5);
        
        // Fuel level from sensor (or mock if not connected)
        doc["fuel_level"] = readFuelLevel();
        doc["engine_hours"] = 1234;
        
        // Add glow plug countdown (whenever glow plugs are on and timer is running)
        bool glowPlugsActive = digitalRead(GLOW_PLUGS_PIN);
        doc["glow_active"] = glowPlugsActive;
        
        if (glowPlugsActive && g_systemState.glowPlugStartTime > 0) {
            unsigned long elapsed = millis() - g_systemState.glowPlugStartTime;
            if (elapsed < GLOW_PLUG_DURATION) {
                unsigned long remaining = (GLOW_PLUG_DURATION - elapsed) / 1000;
                doc["countdown"] = remaining;
            } else {
                doc["countdown"] = 0;  // Timer expired but still on
            }
        } else {
            doc["countdown"] = 0;
        }
        
        // Add key position information
        doc["key_position"] = g_systemState.keyPosition;
        doc["start_key_held"] = g_systemState.keyStartHeld;
        
        // Add power management information
        doc["sleep_mode_enabled"] = g_systemState.sleepModeEnabled;
        unsigned long timeSinceActivity = millis() - g_systemState.lastActivityTime;
        doc["time_since_activity"] = timeSinceActivity / 1000; // Convert to seconds
        doc["sleep_eligible"] = checkSleepConditions();
        doc["time_until_sleep"] = max(0UL, (SLEEP_TIMEOUT - timeSinceActivity) / 1000); // Seconds until sleep
        
        // Add WiFi status information
        doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
        if (WiFi.status() == WL_CONNECTED) {
            doc["wifi_ip"] = WiFi.localIP().toString();
            doc["wifi_ssid"] = WiFi.SSID();
        }
        doc["ap_ip"] = WiFi.softAPIP().toString();
        doc["ap_clients"] = WiFi.softAPgetStationNum();

        String jsonResponse;
        serializeJson(doc, jsonResponse);
        request->send(200, "application/json", jsonResponse);
    });

    // Raw sensor data endpoint for calibration
    server.on("/api/raw-sensors", HTTP_GET, [](AsyncWebServerRequest *request){
        StaticJsonDocument<768> doc;
        
        // Read raw ADC values (0-4095)
        int batteryRaw = analogRead(BATTERY_VOLTAGE_PIN);
        int temperatureRaw = analogRead(ENGINE_TEMP_PIN);
        int pressureRaw = analogRead(OIL_PRESSURE_PIN);
        int fuelRaw = analogRead(FUEL_LEVEL_PIN);
        int hydRaw = analogRead(HYD_PRESSURE_PIN);
        
        doc["battery_raw"] = batteryRaw;
        doc["temperature_raw"] = temperatureRaw;
        doc["pressure_raw"] = pressureRaw;
        doc["fuel_raw"] = fuelRaw;
        doc["hydraulic_raw"] = hydRaw;
        
        // Sensor diagnostics
        doc["battery_status"] = (batteryRaw > 100 && batteryRaw < 4000) ? "OK" : "CHECK";
        doc["temperature_status"] = (temperatureRaw > 10 && temperatureRaw < 4000) ? "OK" : "CHECK";
    doc["pressure_status"] = (pressureRaw < 4000) ? "OK" : "BROKEN";
    doc["hydraulic_status"] = (hydRaw < 4000) ? "OK" : "BROKEN";
        doc["fuel_status"] = (fuelRaw > 10 && fuelRaw < 4090) ? "OK" : "CHECK";
        
        // Detailed diagnostics for pressure sensor
        if (pressureRaw > 4000) {
            doc["pressure_diagnostic"] = "SENSOR DISCONNECTED/BROKEN - Reading mega ohms";
        } else if (pressureRaw < 10) {
            doc["pressure_diagnostic"] = "SENSOR SHORT CIRCUIT - Very low resistance";
        } else {
            doc["pressure_diagnostic"] = "Normal operation";
        }
        
        // Include current runtime calibration constants (from preferences, not static config)
        doc["battery_divider"] = runtime_battery_divider;
        doc["temp_offset"] = TEMP_SENSOR_OFFSET;  // Not calibrated yet
        doc["temp_scale"] = runtime_temp_scale;
        doc["pressure_offset"] = OIL_PRESSURE_OFFSET;  // Not calibrated yet
        doc["pressure_scale"] = runtime_pressure_scale;
        doc["hyd_pressure_scale"] = runtime_hyd_pressure_scale;
        doc["fuel_empty"] = runtime_fuel_empty;
        doc["fuel_full"] = runtime_fuel_full;
        
        // Include calculated values for comparison
        doc["battery_calculated"] = readBatteryVoltage();
        doc["temperature_calculated"] = readEngineTemp();
        doc["pressure_calculated"] = readOilPressure();
        doc["fuel_calculated"] = readFuelLevel();
        doc["hydraulic_calculated"] = readHydraulicPressure();
        
        String jsonResponse;
        serializeJson(doc, jsonResponse);
        request->send(200, "application/json", jsonResponse);
    });

    // Simple calibration endpoint - all logic in C++
    server.on("/api/auto-calibrate", HTTP_POST, [](AsyncWebServerRequest *request){
        StaticJsonDocument<256> responseDoc;
        
        if (!request->hasParam("sensor", true) || !request->hasParam("actual_value", true)) {
            responseDoc["status"] = "error";
            responseDoc["message"] = "Missing sensor or actual_value parameter";
            String jsonResponse;
            serializeJson(responseDoc, jsonResponse);
            request->send(400, "application/json", jsonResponse);
            return;
        }
        
        String sensor = request->getParam("sensor", true)->value();
        float actualValue = request->getParam("actual_value", true)->value().toFloat();
        
        // Get current raw sensor readings
        int batteryRaw = analogRead(BATTERY_VOLTAGE_PIN);
        int temperatureRaw = analogRead(ENGINE_TEMP_PIN);
        int pressureRaw = analogRead(OIL_PRESSURE_PIN);
        int fuelRaw = analogRead(FUEL_LEVEL_PIN);
        int hydRaw = analogRead(HYD_PRESSURE_PIN);
        
        bool calibrationApplied = false;
        String calibrationDetails = "";
        
        Preferences prefs;
        prefs.begin("calibration", false);
        
        if (sensor == "battery" && batteryRaw > 100) {
            // Battery voltage calibration: new_divider = actual_voltage / raw_adc_value
            float newDivider = actualValue / batteryRaw;
            if (newDivider > 0.0001 && newDivider < 1.0) {
                prefs.putFloat("battery_div", newDivider);
                calibrationDetails = "Battery divider: " + String(newDivider, 6);
                calibrationApplied = true;
            }
        }
        else if (sensor == "temperature" && temperatureRaw > 10) {
            // Temperature calibration: scale = (150 - actual_temp) / raw_adc (inverted NTC)
            float baseTemp = 150.0;
            float newScale = (baseTemp - actualValue) / temperatureRaw;
            if (newScale > 0.001 && newScale < 10.0) {
                prefs.putFloat("temp_scale", newScale);
                calibrationDetails = "Temperature scale: " + String(newScale, 6);
                calibrationApplied = true;
            }
        }
        else if (sensor == "pressure" && pressureRaw < 4000) {
            // Pressure calibration: scale = actual_pressure / raw_adc
            float newScale = actualValue / pressureRaw;
            if (newScale > 0.001 && newScale < 10.0) {
                prefs.putFloat("pressure_scale", newScale);
                calibrationDetails = "Pressure scale: " + String(newScale, 6);
                calibrationApplied = true;
            }
        }
        else if (sensor == "hydraulic" && hydRaw < 4000) {
            float newScale = actualValue / hydRaw;
            if (newScale > 0.001 && newScale < 10.0) {
                prefs.putFloat("hyd_pressure_scale", newScale);
                calibrationDetails = "Hydraulic pressure scale: " + String(newScale, 6);
                calibrationApplied = true;
            }
        }
        else if (sensor == "fuel" && fuelRaw > 10 && fuelRaw < 4090) {
            // Fuel level calibration
            if (actualValue <= 10) {
                // Low fuel reading - set as empty
                prefs.putInt("fuel_empty", fuelRaw);
                calibrationDetails = "Fuel empty: " + String(fuelRaw);
                calibrationApplied = true;
            } else if (actualValue >= 90) {
                // High fuel reading - set as full
                prefs.putInt("fuel_full", fuelRaw);
                calibrationDetails = "Fuel full: " + String(fuelRaw);
                calibrationApplied = true;
            } else {
                // Mid-range - interpolate between current empty/full
                int currentEmpty = prefs.getInt("fuel_empty", (int)FUEL_LEVEL_EMPTY);
                int currentFull = prefs.getInt("fuel_full", (int)FUEL_LEVEL_FULL);
                
                // Calculate what the ADC should be for this percentage
                float expectedADC = currentEmpty + (actualValue / 100.0) * (currentFull - currentEmpty);
                float scaleFactor = expectedADC / fuelRaw;
                
                // Scale both empty and full proportionally
                int newEmpty = (int)(currentEmpty * scaleFactor);
                int newFull = (int)(currentFull * scaleFactor);
                
                prefs.putInt("fuel_empty", newEmpty);
                prefs.putInt("fuel_full", newFull);
                calibrationDetails = "Fuel empty: " + String(newEmpty) + ", full: " + String(newFull);
                calibrationApplied = true;
            }
        }
        
        prefs.end();
        
        if (calibrationApplied) {
            // Reload calibration constants immediately
            loadCalibrationConstants();
            responseDoc["status"] = "success";
            responseDoc["message"] = "Calibration applied: " + calibrationDetails;
        } else {
            responseDoc["status"] = "error";
            responseDoc["message"] = "Calibration failed - invalid sensor data or value out of range";
        }
        
        String jsonResponse;
        serializeJson(responseDoc, jsonResponse);
        request->send(200, "application/json", jsonResponse);
    });

    // Update calibration constants endpoint
    server.on("/api/calibration", HTTP_POST, [](AsyncWebServerRequest *request){
        StaticJsonDocument<512> responseDoc;
        bool updated = false;
        String updatedConstants = "";
        
        // Handle form-encoded data
        if (request->hasParam("battery_divider", true)) {
            String value = request->getParam("battery_divider", true)->value();
            float newDivider = value.toFloat();
            if (newDivider > 0.0001 && newDivider < 1.0) { // Much wider range
                // Store in preferences for next restart
                Preferences prefs;
                prefs.begin("calibration", false);
                prefs.putFloat("battery_div", newDivider);
                prefs.end();
                updatedConstants += "Battery divider: " + String(newDivider, 6) + " ";
                updated = true;
            }
        }
        
        if (request->hasParam("temp_scale", true)) {
            String value = request->getParam("temp_scale", true)->value();
            float newScale = value.toFloat();
            if (newScale > 0.001 && newScale < 10.0) { // Much wider range
                Preferences prefs;
                prefs.begin("calibration", false);
                prefs.putFloat("temp_scale", newScale);
                prefs.end();
                updatedConstants += "Temp scale: " + String(newScale, 6) + " ";
                updated = true;
            }
        }
        
        if (request->hasParam("pressure_scale", true)) {
            String value = request->getParam("pressure_scale", true)->value();
            float newScale = value.toFloat();
            if (newScale > 0.001 && newScale < 10.0) { // Much wider range
                Preferences prefs;
                prefs.begin("calibration", false);
                prefs.putFloat("pressure_scale", newScale);
                prefs.end();
                updatedConstants += "Pressure scale: " + String(newScale, 6) + " ";
                updated = true;
            }
        }
        if (request->hasParam("hyd_pressure_scale", true)) {
            String value = request->getParam("hyd_pressure_scale", true)->value();
            float newScale = value.toFloat();
            if (newScale > 0.001 && newScale < 10.0) {
                Preferences prefs;
                prefs.begin("calibration", false);
                prefs.putFloat("hyd_pressure_scale", newScale);
                prefs.end();
                updatedConstants += "Hydraulic pressure scale: " + String(newScale, 6) + " ";
                updated = true;
            }
        }
        
        if (request->hasParam("fuel_empty", true)) {
            String value = request->getParam("fuel_empty", true)->value();
            int newEmpty = value.toInt();
            if (newEmpty >= 0 && newEmpty <= 4095) {
                Preferences prefs;
                prefs.begin("calibration", false);
                prefs.putInt("fuel_empty", newEmpty);
                prefs.end();
                updatedConstants += "Fuel empty: " + String(newEmpty) + " ";
                updated = true;
            }
        }
        
        if (request->hasParam("fuel_full", true)) {
            String value = request->getParam("fuel_full", true)->value();
            int newFull = value.toInt();
            if (newFull >= 0 && newFull <= 4095) {
                Preferences prefs;
                prefs.begin("calibration", false);
                prefs.putInt("fuel_full", newFull);
                prefs.end();
                updatedConstants += "Fuel full: " + String(newFull) + " ";
                updated = true;
            }
        }
        
        if (updated) {
            // Reload calibration constants immediately to apply changes
            loadCalibrationConstants();
            responseDoc["status"] = "success";
            responseDoc["message"] = "Calibration updated: " + updatedConstants + "Applied immediately.";
        } else {
            responseDoc["status"] = "error";
            responseDoc["message"] = "No valid calibration parameters provided or values out of range";
        }
        
        String jsonResponse;
        serializeJson(responseDoc, jsonResponse);
        request->send(200, "application/json", jsonResponse);
    });

    // Reset calibration to defaults
    server.on("/api/reset-calibration", HTTP_POST, [](AsyncWebServerRequest *request){
        StaticJsonDocument<256> doc;
        Preferences prefs;
        prefs.begin("calibration", false);
        // Clear all calibration keys in this namespace
        prefs.clear();
        prefs.end();
        // Reload runtime constants from defaults (or stored values if any)
        loadCalibrationConstants();
        doc["status"] = "success";
        doc["message"] = "Calibration reset to defaults";
        String jsonResponse;
        serializeJson(doc, jsonResponse);
        request->send(200, "application/json", jsonResponse);
    });

    // WiFi information endpoint
    server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request){
        StaticJsonDocument<256> doc;
        doc["mode"] = "AP_STA";
        doc["home_connected"] = (WiFi.status() == WL_CONNECTED);
        if (WiFi.status() == WL_CONNECTED) {
            doc["home_ip"] = WiFi.localIP().toString();
            doc["home_ssid"] = WiFi.SSID();
            doc["home_rssi"] = WiFi.RSSI();
        }
        doc["ap_ssid"] = ssid;
        doc["ap_ip"] = WiFi.softAPIP().toString();
        doc["ap_clients"] = WiFi.softAPgetStationNum();
        
        String jsonResponse;
        serializeJson(doc, jsonResponse);
        request->send(200, "application/json", jsonResponse);
    });

    // Settings page endpoint
    server.on("/settings.html", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/settings.html", "text/html");
    });

    server.on("/settings.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/settings.js", "text/javascript");
    });

    // Settings API - Get current settings
    server.on("/api/settings", HTTP_GET, handleGetSettings);

    // Settings API - Save settings
    server.on("/api/settings", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, onSetSettingBody);

    // OTA Update endpoint - redirect to ElegantOTA
    server.on("/api/ota-update", HTTP_POST, [](AsyncWebServerRequest *request){
        Serial.println("OTA update requested - ElegantOTA available at /update");
        request->send(200, "application/json", "{\"success\":true,\"message\":\"OTA interface available at /update\"}");
    });

    // Factory Reset endpoint
    server.on("/api/factory-reset", HTTP_POST, [](AsyncWebServerRequest *request){
        Serial.println("Factory reset requested via web interface");
        request->send(200, "text/plain", "Factory reset initiated");
        
        // Perform factory reset
        if (g_settingsManager.performFactoryReset()) {
            Serial.println("Factory reset completed - restarting device");
            delay(1000);
            ESP.restart();
        } else {
            Serial.println("Factory reset failed");
        }
    });

    // Initialize ElegantOTA
    ElegantOTA.begin(&server);

    // Start the server
    server.begin();
    Serial.println("Web server started");
}
