/*
 * Bobcat Ignition Controller - Main File
 * ESP32-based system to control Bobcat ignition sequence
 * 
 * Features:
 * - Glow plug preheating (20 seconds)
 * - Main ignition control
 * - Safety interlocks
 * - Status monitoring
 * 
 * Author: Generated for Bobcat Controller Project
 * Date: June 2025
 */

#include <Arduino.h>
#include "config.h"
#include "hardware.h"
#include "safety.h"
#include "system_state.h"
#include "web_interface.h"
#include "settings.h"
#include <ElegantOTA.h>
// Optional CLI-friendly OTA (PlatformIO espota.py)
#include <ArduinoOTA.h>
#include <WiFi.h>

static bool g_otaInitialized = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Bobcat Ignition Controller Starting...");
  
  // Check if this is a wake-up from deep sleep
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason != ESP_SLEEP_WAKEUP_UNDEFINED) {
    handleWakeUp();
  } else {
    Serial.println("Cold boot - initializing normally...");
  }
  
  // Initialize settings manager first
  if (!g_settingsManager.begin()) {
    Serial.println("WARNING: Settings manager failed to initialize, using defaults");
  }
  
  initializePins();
  initializeSleepMode(); // Initialize deep sleep functionality
  
  g_systemState.currentState = OFF;  // Start in OFF state like a real ignition
  g_systemState.keyPosition = 0;     // Key starts in OFF position
  
  setupWebServer(); // Initialize the web server

  // Configure ArduinoOTA (begin is deferred until WiFi connected)
  ArduinoOTA.setHostname("bobcat-ignition");
  // To require a password, uncomment and pass via PIO with --auth
  // ArduinoOTA.setPassword("change_me");
  ArduinoOTA.onStart([]() { Serial.println("ArduinoOTA: Start"); });
  ArduinoOTA.onEnd([]() { Serial.println("ArduinoOTA: End"); });
  ArduinoOTA.onError([](ota_error_t error) { Serial.printf("ArduinoOTA Error[%u]\n", error); });

  Serial.println("System initialized - Key is in OFF position");
  Serial.println("Turn key to ON, then GLOW PLUG, then hold START to crank engine");
  Serial.println("System will auto-sleep after 30 minutes of inactivity");
}

void loop() {
  // ElegantOTA loop function
  ElegantOTA.loop();
  // Handle ArduinoOTA in the main loop (non-blocking)
  if (!g_otaInitialized && WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.begin();
    g_otaInitialized = true;
    Serial.print("ArduinoOTA listening on ");
    Serial.print(WiFi.localIP());
    Serial.println(":3232");
  }
  ArduinoOTA.handle();
  
  // Don't allow automatic sleep for the first 2 minutes after boot
  // This gives time to connect and configure the system
  unsigned long currentTime = millis();
  const unsigned long BOOT_GRACE_PERIOD = 120000; // 2 minutes
  
  if (currentTime < BOOT_GRACE_PERIOD) {
    // During grace period, just update activity timer to keep system awake
    g_systemState.lastActivityTime = currentTime;
  } else {
    // After grace period, check for sleep conditions and enter sleep if appropriate
    if (checkSleepConditions()) {
      unsigned long timeSinceActivity = currentTime - g_systemState.lastActivityTime;
      
      if (timeSinceActivity >= SLEEP_TIMEOUT) {
        Serial.println("Sleep timeout reached - entering deep sleep mode");
        enterDeepSleep();
        // Code will not reach here as ESP32 will sleep
      }
    }
  }
  
  // Main ignition key sequence control
  runIgnitionSequence();
  
  // Only check engine vitals when running
  if (g_systemState.currentState == RUNNING) { 
    checkEngineVitals();
  } 
  
  // Always run safety checks except during start cranking
  if (g_systemState.currentState != START) {
    checkSafetyInputs();
  }
  
  // Small delay to prevent overwhelming the system
  delay(10);
}
