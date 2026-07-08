/*
 * Bobcat Ignition Controller - Main File
 * ESP32 controller for a Bobcat 743 / Kubota V1702 diesel.
 *
 * loop() owns ALL relay actuation, flash writes, and sleep. Web handlers only
 * mutate state / set flags (see system_state.h cross-task flags).
 */

#include <Arduino.h>
#include "config.h"
#include "hardware.h"
#include "safety.h"
#include "system_state.h"
#include "web_interface.h"
#include "settings.h"
#include "mqtt_handler.h"
#include <ElegantOTA.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

static bool g_otaInitialized = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Bobcat Ignition Controller Starting...");

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason != ESP_SLEEP_WAKEUP_UNDEFINED) {
    handleWakeUp();
  } else {
    Serial.println("Cold boot - initializing normally...");
  }

  // Settings first: the pin map + calibration are read from here.
  if (!g_settingsManager.begin()) {
    Serial.println("WARNING: Settings manager failed to initialize, using defaults");
  }

  initializePins();        // applies pin map + calibration from settings
  initializeSleepMode();

  g_systemState.currentState = OFF;
  g_systemState.keyPosition = 0;

  setupWebServer();

  // MQTT telemetry (read-only). Constructs the client only; loopTask connects.
  mqttInit();

  ArduinoOTA.setHostname("bobcat-ignition");
  ArduinoOTA.onStart([]() { Serial.println("ArduinoOTA: Start"); });
  ArduinoOTA.onEnd([]() { Serial.println("ArduinoOTA: End"); });
  ArduinoOTA.onError([](ota_error_t error) { Serial.printf("ArduinoOTA Error[%u]\n", error); });

  Serial.println("System initialized - Key is in OFF position");
}

void loop() {
  ElegantOTA.loop();
  if (!g_otaInitialized && WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.begin();
    g_otaInitialized = true;
    Serial.print("ArduinoOTA listening on ");
    Serial.print(WiFi.localIP());
    Serial.println(":3232");
  }
  ArduinoOTA.handle();

  // MQTT telemetry: ALL PubSubClient work happens here in loopTask.
  mqttLoop();

  unsigned long now = millis();

  // ---- Deferred work requested by async web handlers (flash / sleep / crank) ----
  if (g_systemState.configDirty) {
    g_settingsManager.saveSettings();
    g_systemState.configDirty = false;
  }
  if (g_systemState.reloadCalibration) {
    loadCalibrationConstants();
    g_systemState.reloadCalibration = false;
  }
  if (g_systemState.wifiReconnect) {
    g_systemState.wifiReconnect = false;
    const BobcatSettings& ws = g_settingsManager.getSettings();
    if (strlen(ws.wifiSSID) > 0) {
      Serial.printf("WiFi reconnect to '%s'...\n", ws.wifiSSID);
      WiFi.disconnect();
      WiFi.begin(ws.wifiSSID, ws.wifiPassword);  // non-blocking; AP stays up (AP_STA)
      g_otaInitialized = false;                  // re-arm ArduinoOTA on new IP
    }
  }
  if (g_systemState.overrideRequested) {
    g_systemState.overrideRequested = false;
    overrideStart();   // enforces interlocks + battery internally
  }
  if (g_systemState.factoryResetRequested) {
    g_systemState.factoryResetRequested = false;
    Serial.println("Factory reset (from loop) - restarting device");
    g_settingsManager.performFactoryReset();
    delay(500);
    ESP.restart();
  }
  if (g_systemState.sleepRequested) {
    g_systemState.sleepRequested = false;
    if (checkSleepConditions(true)) {
      enterDeepSleep();  // does not return
    } else {
      Serial.println("Sleep request denied - unsafe conditions");
    }
  }

  // ---- Auto-sleep ----
  const unsigned long BOOT_GRACE_PERIOD = 120000;
  if (now < BOOT_GRACE_PERIOD) {
    g_systemState.lastActivityTime = now;
  } else {
    if (checkSleepConditions()) {
      unsigned long timeSinceActivity = now - g_systemState.lastActivityTime;
      if (timeSinceActivity >= SLEEP_TIMEOUT) {
        Serial.println("Sleep timeout reached - entering deep sleep");
        enterDeepSleep();
      }
    }
  }

  // ---- State machine (all relay actuation happens here) ----
  runIgnitionSequence();

  // ---- Engine vitals while running (over-temp / low-oil alerts) ----
  if (isEngineRunning()) {
    checkEngineVitals();
  }

  // ---- General safety checks (skip during active cranking) ----
  if (g_systemState.currentState != START) {
    checkSafetyInputs();
  }

  // ---- Engine-hours accumulation ----
  updateEngineHours(now);

  delay(10);
}
