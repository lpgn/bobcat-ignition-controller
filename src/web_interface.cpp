/*
 * Web Interface Implementation for Bobcat Ignition Controller
 *
 * Async-safe: handlers set flags / mutate RAM only. loop() performs all flash
 * writes (configDirty), relay actuation (state machine), override cranks and
 * deep sleep (see system_state.h flags).
 */

#include "web_interface.h"
#include "config.h"
#include "hardware.h"
#include "system_state.h"
#include "safety.h"
#include "settings.h"
#include "mqtt_handler.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>
#include <math.h>

static AsyncWebServer server(80);

// ---------------------------------------------------------------------------
// Small reply helpers
// ---------------------------------------------------------------------------
static void sendOk(AsyncWebServerRequest* req) {
  req->send(200, "application/json", "{\"ok\":true}");
}

static void sendErr(AsyncWebServerRequest* req, int code, const String& msg) {
  StaticJsonDocument<192> r;
  r["ok"] = false;
  r["error"] = msg;
  String out;
  serializeJson(r, out);
  req->send(code, "application/json", out);
}

static bool canCrank(String& err) {
  if (g_systemState.maintenanceMode) return true;   // test/maintenance: gates bypassed
  if (!safetyInterlocksPassed()) { err = "interlocks not satisfied (seat bar + neutral)"; return false; }
  if (!batteryOkToStart())       { err = "battery voltage too low to crank"; return false; }
  return true;
}

// ---------------------------------------------------------------------------
// GET /api/status
// ---------------------------------------------------------------------------
static void buildStatus(JsonDocument& json) {
  const BobcatSettings& s = g_settingsManager.getSettings();
  int st = g_systemState.currentState;

  json["state"] = apiStateName(st);
  json["seq"] = apiStateSeq(st);
  json["maintenance"] = g_systemState.maintenanceMode;
  json["engineHours"] = g_settingsManager.getEngineHours() +
                        g_systemState.engineHoursAccumMs / 3600000.0;

  float temp = readEngineTemp();
  float oil  = readOilPressure();
  float hyd  = readHydraulicPressure();
  float batt = readBatteryVoltage();
  float fuel = readFuelLevel();

  JsonObject h = json.createNestedObject("hydraulic");
  h["v"] = (int)lroundf(hyd);
  h["min"] = s.minHydPressure;
  h["max"] = HYD_DISPLAY_MAX;
  h["unit"] = "psi";

  JsonObject et = json.createNestedObject("engineTemp");
  et["v"] = (int)lroundf(temp);
  et["min"] = TEMP_DISPLAY_MIN;
  et["max"] = TEMP_DISPLAY_MAX;
  et["warn"] = s.maxCoolantTemp;
  et["crit"] = s.maxCoolantTemp + 6;
  et["unit"] = "C";

  JsonObject o = json.createNestedObject("oil");
  o["v"] = (int)lroundf(oil);
  o["min"] = s.minOilPressure;
  o["max"] = OIL_DISPLAY_MAX;
  o["unit"] = "psi";

  JsonObject b = json.createNestedObject("battery");
  b["v"] = lroundf(batt * 10.0f) / 10.0;
  b["min"] = s.minBatteryVoltage;
  b["max"] = s.maxBatteryVoltage;
  b["unit"] = "V";

  JsonObject f = json.createNestedObject("fuel");
  f["v"] = (int)lroundf(fuel);
  f["low"] = s.fuelLevelLowThreshold;
  f["unit"] = "%";

  JsonObject out = json.createNestedObject("outputs");
  out["power"]   = outputOn(PIN_MAIN_POWER);
  out["glow"]    = outputOn(PIN_GLOW);
  out["starter"] = outputOn(PIN_STARTER);
  out["lights"]  = outputOn(PIN_LIGHTS);

  bool running = isEngineRunning();
  JsonObject flt = json.createNestedObject("faults");
  flt["oil"]     = running && (oil < s.minOilPressure);
  flt["temp"]    = temp > s.maxCoolantTemp;
  flt["battery"] = batt < s.minBatteryVoltage;
  flt["fuel"]    = fuel < s.fuelLevelLowThreshold;

  JsonObject g = json.createNestedObject("glow");
  bool glowActive = outputOn(PIN_GLOW);
  g["active"] = glowActive;
  int countdown = 0;
  if (glowActive && g_systemState.glowPlugStartTime > 0) {
    unsigned long elapsed = millis() - g_systemState.glowPlugStartTime;
    if (elapsed < s.glowPlugDuration) countdown = (s.glowPlugDuration - elapsed) / 1000;
  }
  g["countdown"] = countdown;

  JsonObject net = json.createNestedObject("net");
  bool wifi = (WiFi.status() == WL_CONNECTED);
  net["wifi"] = wifi;
  net["ip"] = wifi ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
  net["rssi"] = wifi ? WiFi.RSSI() : 0;
  net["apClients"] = (int)WiFi.softAPgetStationNum();
  net["mqtt"] = mqttConnected(); // live MQTT connection state (cached, thread-safe)
  net["clock"] = "--:--";       // NTP stub
}

static void handleApiStatus(AsyncWebServerRequest* request) {
  DynamicJsonDocument doc(1536);
  buildStatus(doc);
  String out;
  serializeJson(doc, out);
  request->send(200, "application/json", out);
}

// ---------------------------------------------------------------------------
// POST /api/control  {"action":...}
// ---------------------------------------------------------------------------
static void onControlBody(AsyncWebServerRequest* request, uint8_t* data, size_t len,
                          size_t index, size_t total) {
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, data, len)) { sendErr(request, 400, "invalid JSON"); return; }

  updateActivityTimer();
  const char* action = doc["action"] | "";

  if (strcmp(action, "key") == 0) {
    if (!doc.containsKey("position")) { sendErr(request, 400, "missing position"); return; }
    int pos = doc["position"] | -1;
    if (pos < 0 || pos > 3) { sendErr(request, 400, "position must be 0..3"); return; }
    g_systemState.keyPosition = pos;      // pos 3 (crank) glows always; starter gated in START
    if (pos < 3) g_systemState.keyStartHeld = false;
    sendOk(request);
    return;
  }

  if (strcmp(action, "crank") == 0) {
    bool held = doc["held"] | false;
    if (held) {
      // Always accept: glow runs on hold. The STARTER is gated (interlocks + battery)
      // in the START state; report whether it will actually crank right now.
      g_systemState.keyStartHeld = true;
      g_systemState.keyPosition = 3;
      g_systemState.startHoldTime = millis();
      String err;
      bool starter = canCrank(err);
      StaticJsonDocument<128> r;
      r["ok"] = true;
      r["starter"] = starter;
      if (!starter) r["note"] = err;
      String out; serializeJson(r, out);
      request->send(200, "application/json", out);
    } else {
      g_systemState.keyStartHeld = false;
      if (g_systemState.keyPosition >= 3) g_systemState.keyPosition = 1;
      sendOk(request);
    }
    return;
  }

  if (strcmp(action, "stop") == 0) {
    // Cut starter + glow, keep power (engine is stopped by the fuel lever).
    g_systemState.emergencyStopPressed = true;
    sendOk(request);
    return;
  }

  if (strcmp(action, "lights") == 0) {
    g_systemState.lightsTogglePressed = true;
    sendOk(request);
    return;
  }

  if (strcmp(action, "override") == 0) {
    // Deliberate override: bypasses sensor faults, but interlocks + battery are
    // still enforced (checked here for feedback and again in loop()).
    String err;
    if (!canCrank(err)) { sendErr(request, 400, err); return; }
    g_systemState.overrideRequested = true;
    sendOk(request);
    return;
  }

  if (strcmp(action, "maintenance") == 0) {
    bool en = doc["enabled"] | false;
    g_systemState.maintenanceMode = en;
    if (en) g_systemState.maintenanceStart = millis();
    Serial.println(en ? "MAINTENANCE MODE ON - interlocks + battery bypassed"
                      : "Maintenance mode OFF");
    sendOk(request);
    return;
  }

  sendErr(request, 400, String("unknown action: ") + action);
}

// ---------------------------------------------------------------------------
// GET /api/settings
// ---------------------------------------------------------------------------
static void handleGetSettings(AsyncWebServerRequest* request) {
  DynamicJsonDocument doc(1024);
  const BobcatSettings& s = g_settingsManager.getSettings();

  JsonObject engine = doc.createNestedObject("engine");
  engine["glow"] = s.glowPlugDuration / 1000;
  engine["crank"] = s.crankingTimeout / 1000;
  engine["cooldown"] = s.cooldownDuration / 1000;
  engine["glowAssist"] = s.glowAssistDuration / 1000;

  JsonObject th = doc.createNestedObject("thresholds");
  th["maxTemp"] = s.maxCoolantTemp;
  th["minOil"] = s.minOilPressure;
  th["minHyd"] = s.minHydPressure;
  th["minV"] = s.minBatteryVoltage;
  th["maxV"] = s.maxBatteryVoltage;

  JsonObject cal = doc.createNestedObject("cal");
  cal["tempScale"] = s.tempSensorScale;
  cal["oilScale"] = s.oilPressureScale;
  cal["hydScale"] = s.hydPressureScale;
  cal["fuelEmpty"] = s.fuelLevelEmpty;
  cal["fuelFull"] = s.fuelLevelFull;
  cal["fuelLow"] = s.fuelLevelLowThreshold;

  JsonObject mqtt = doc.createNestedObject("mqtt");
  mqtt["enabled"] = (bool)s.mqttEnabled;
  mqtt["host"] = s.mqttHost;
  mqtt["port"] = s.mqttPort;
  mqtt["topic"] = s.mqttTopic;

  JsonObject tm = doc.createNestedObject("time");
  tm["ntp"] = (bool)s.ntpEnabled;
  tm["utcOffset"] = s.utcOffset;

  JsonObject wifi = doc.createNestedObject("wifi");
  JsonArray nets = wifi.createNestedArray("networks");
  if (strlen(s.wifiSSID) > 0) nets.add(s.wifiSSID);
  JsonObject ap = wifi.createNestedObject("ap");
  ap["ssid"] = s.apSSID;

  String out;
  serializeJson(doc, out);
  request->send(200, "application/json", out);
}

// ---------------------------------------------------------------------------
// POST /api/settings  {"key":...,"value":...}
// ---------------------------------------------------------------------------
static void onSettingsBody(AsyncWebServerRequest* request, uint8_t* data, size_t len,
                           size_t index, size_t total) {
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, data, len)) { sendErr(request, 400, "invalid JSON"); return; }

  const char* key = doc["key"] | "";
  JsonVariant v = doc["value"];
  if (!key[0] || v.isNull()) { sendErr(request, 400, "missing key or value"); return; }

  const BobcatSettings& c = g_settingsManager.getSettings();
  bool ok = false;
  bool calChanged = false;
  String err = "invalid value";

  auto keyIs = [&](const char* a, const char* b) {
    return strcmp(key, a) == 0 || (b && strcmp(key, b) == 0);
  };

  if (keyIs("glow", "glowDuration")) {
    ok = g_settingsManager.updateEngineSettings((uint32_t)(v.as<uint32_t>() * 1000),
                                                c.crankingTimeout, c.cooldownDuration);
  } else if (keyIs("crank", "crankingTimeout")) {
    ok = g_settingsManager.updateEngineSettings(c.glowPlugDuration,
                                                (uint32_t)(v.as<uint32_t>() * 1000), c.cooldownDuration);
  } else if (keyIs("cooldown", "cooldownDuration")) {
    ok = g_settingsManager.updateEngineSettings(c.glowPlugDuration, c.crankingTimeout,
                                                (uint32_t)(v.as<uint32_t>() * 1000));
  } else if (keyIs("glowAssist", nullptr)) {
    ok = g_settingsManager.setGlowAssist((uint32_t)(v.as<uint32_t>() * 1000));
  } else if (keyIs("maxTemp", nullptr)) {
    ok = g_settingsManager.updateAlarmThresholds(v.as<int16_t>(), c.minOilPressure,
                                                 c.minBatteryVoltage, c.maxBatteryVoltage);
  } else if (keyIs("minOil", "minOilPressure")) {
    ok = g_settingsManager.updateAlarmThresholds(c.maxCoolantTemp, v.as<int16_t>(),
                                                 c.minBatteryVoltage, c.maxBatteryVoltage);
  } else if (keyIs("minHyd", "minHydPressure")) {
    ok = g_settingsManager.updateHydraulicThreshold(v.as<int16_t>());
  } else if (keyIs("minV", "minVoltage")) {
    ok = g_settingsManager.updateAlarmThresholds(c.maxCoolantTemp, c.minOilPressure,
                                                 v.as<float>(), c.maxBatteryVoltage);
  } else if (keyIs("maxV", "maxVoltage")) {
    ok = g_settingsManager.updateAlarmThresholds(c.maxCoolantTemp, c.minOilPressure,
                                                 c.minBatteryVoltage, v.as<float>());
  } else if (keyIs("tempScale", nullptr)) {
    ok = g_settingsManager.setTempScale(v.as<float>()); calChanged = ok;
  } else if (keyIs("oilScale", nullptr)) {
    ok = g_settingsManager.setOilScale(v.as<float>()); calChanged = ok;
  } else if (keyIs("hydScale", nullptr)) {
    ok = g_settingsManager.setHydScale(v.as<float>()); calChanged = ok;
  } else if (keyIs("batteryDivider", nullptr)) {
    ok = g_settingsManager.setBatteryDivider(v.as<float>()); calChanged = ok;
  } else if (keyIs("fuelEmpty", nullptr)) {
    ok = g_settingsManager.setFuelEmpty(v.as<uint16_t>()); calChanged = ok;
  } else if (keyIs("fuelFull", nullptr)) {
    ok = g_settingsManager.setFuelFull(v.as<uint16_t>()); calChanged = ok;
  } else if (keyIs("fuelLow", "fuelLevelLowThreshold")) {
    ok = g_settingsManager.setFuelLowThreshold(v.as<uint8_t>());
  } else if (keyIs("wifiSSID", nullptr)) {
    ok = g_settingsManager.updateWifiSettings(v.as<const char*>(), c.wifiPassword);
  } else if (keyIs("wifiPassword", nullptr)) {
    ok = g_settingsManager.updateWifiSettings(c.wifiSSID, v.as<const char*>());
  } else if (keyIs("apSSID", nullptr)) {
    ok = g_settingsManager.updateApSettings(v.as<const char*>(), c.apPassword);
  } else if (keyIs("apPassword", nullptr)) {
    ok = g_settingsManager.updateApSettings(c.apSSID, v.as<const char*>());
  } else if (keyIs("mqttEnabled", nullptr)) {
    ok = g_settingsManager.setMqttEnabled(v.as<bool>() ? 1 : 0);
  } else if (keyIs("mqttHost", nullptr)) {
    ok = g_settingsManager.updateMqtt(c.mqttEnabled, v.as<const char*>(), c.mqttPort, c.mqttTopic);
  } else if (keyIs("mqttPort", nullptr)) {
    ok = g_settingsManager.updateMqtt(c.mqttEnabled, c.mqttHost, v.as<uint16_t>(), c.mqttTopic);
  } else if (keyIs("mqttTopic", nullptr)) {
    ok = g_settingsManager.updateMqtt(c.mqttEnabled, c.mqttHost, c.mqttPort, v.as<const char*>());
  } else if (keyIs("ntp", nullptr)) {
    ok = g_settingsManager.updateTime(v.as<bool>() ? 1 : 0, c.utcOffset);
  } else if (keyIs("utcOffset", nullptr)) {
    ok = g_settingsManager.updateTime(c.ntpEnabled, v.as<int32_t>());
  } else {
    sendErr(request, 400, String("unknown key: ") + key);
    return;
  }

  if (!ok) { sendErr(request, 400, String("rejected value for ") + key); return; }

  g_systemState.configDirty = true;                 // loop() persists to NVS
  if (calChanged) g_systemState.reloadCalibration = true;
  if (strcmp(key, "wifiSSID") == 0 || strcmp(key, "wifiPassword") == 0)
    g_systemState.wifiReconnect = true;             // loop() re-begins STA, no reboot
  if (strncmp(key, "mqtt", 4) == 0)
    mqttRequestReconnect();                          // loopTask re-applies MQTT config
  sendOk(request);
}

// ---------------------------------------------------------------------------
// GET /api/pins  and  POST /api/pins
// ---------------------------------------------------------------------------
static void handleGetPins(AsyncWebServerRequest* request) {
  DynamicJsonDocument doc(2048);

  JsonArray header = doc.createNestedArray("header");
  for (size_t i = 0; i < HEADER_PINS_COUNT; i++) header.add(HEADER_PINS[i]);
  JsonArray adc1 = doc.createNestedArray("adc1");
  for (size_t i = 0; i < ADC1_PINS_COUNT; i++) adc1.add(ADC1_PINS[i]);
  JsonArray relays = doc.createNestedArray("relays");
  for (size_t i = 0; i < RELAY_PINS_COUNT; i++) relays.add(RELAY_PINS[i]);
  JsonArray strap = doc.createNestedArray("strap");
  for (size_t i = 0; i < STRAP_PINS_COUNT; i++) strap.add(STRAP_PINS[i]);

  JsonArray map = doc.createNestedArray("map");
  for (int i = 0; i < PIN_FUNC_COUNT; i++) {
    JsonObject e = map.createNestedObject();
    e["func"] = PIN_TABLE[i].func;
    e["label"] = PIN_TABLE[i].label;
    e["gpio"] = g_settingsManager.getPinGpio(i);
    e["type"] = pinTypeToString(PIN_TABLE[i].type);
  }

  String out;
  serializeJson(doc, out);
  request->send(200, "application/json", out);
}

static int pinFuncIndex(const char* name) {
  for (int i = 0; i < PIN_FUNC_COUNT; i++) {
    if (strcmp(PIN_TABLE[i].func, name) == 0) return i;
  }
  return -1;
}

static void onPinsBody(AsyncWebServerRequest* request, uint8_t* data, size_t len,
                       size_t index, size_t total) {
  StaticJsonDocument<192> doc;
  if (deserializeJson(doc, data, len)) { sendErr(request, 400, "invalid JSON"); return; }

  const char* func = doc["func"] | "";
  if (!func[0] || !doc.containsKey("gpio")) { sendErr(request, 400, "missing func or gpio"); return; }
  int idx = pinFuncIndex(func);
  if (idx < 0) { sendErr(request, 400, String("unknown func: ") + func); return; }

  int gpio = doc["gpio"] | -1;
  if (gpio < 0 || gpio > 39) { sendErr(request, 400, "gpio out of range"); return; }

  String err;
  if (!g_settingsManager.updatePinMap(idx, (uint8_t)gpio, err)) {
    sendErr(request, 400, err);
    return;
  }

  g_systemState.configDirty = true;   // persisted by loop(); applies on reboot

  StaticJsonDocument<96> r;
  r["ok"] = true;
  r["reboot"] = true;                 // new map takes effect after reboot
  String out;
  serializeJson(r, out);
  request->send(200, "application/json", out);
}

// ---------------------------------------------------------------------------
// Legacy status endpoint (kept for the old dashboard) - real values.
// ---------------------------------------------------------------------------
static void handleLegacyStatus(AsyncWebServerRequest* request) {
  DynamicJsonDocument doc(768);
  const BobcatSettings& s = g_settingsManager.getSettings();
  doc["state"] = systemStateToString(g_systemState.currentState);
  doc["engine_temp"] = readEngineTemp();
  doc["oil_pressure"] = readOilPressure();
  doc["hyd_pressure"] = readHydraulicPressure();
  doc["battery_voltage"] = readBatteryVoltage();
  doc["fuel_level"] = readFuelLevel();
  doc["engine_hours"] = g_settingsManager.getEngineHours() +
                        g_systemState.engineHoursAccumMs / 3600000.0;
  doc["main_power_on"] = outputOn(PIN_MAIN_POWER);
  doc["glow_plugs_on"] = outputOn(PIN_GLOW);
  doc["starter_on"] = outputOn(PIN_STARTER);
  doc["lights_on"] = outputOn(PIN_LIGHTS);
  doc["low_battery"] = (readBatteryVoltage() < s.minBatteryVoltage);
  doc["key_position"] = g_systemState.keyPosition;
  doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  doc["ap_clients"] = (int)WiFi.softAPgetStationNum();
  String out;
  serializeJson(doc, out);
  request->send(200, "application/json", out);
}

// ---------------------------------------------------------------------------
// Raw sensor readout (read-only) - for calibration UIs.
// ---------------------------------------------------------------------------
static void handleRawSensors(AsyncWebServerRequest* request) {
  DynamicJsonDocument doc(768);
  doc["battery_raw"] = analogRead(pinFor(PIN_BATTERY));
  doc["temperature_raw"] = analogRead(pinFor(PIN_ENGINE_TEMP));
  doc["oil_raw"] = analogRead(pinFor(PIN_OIL_PRESSURE));
  doc["hydraulic_raw"] = analogRead(pinFor(PIN_HYDRAULIC));
  doc["fuel_raw"] = analogRead(pinFor(PIN_FUEL));

  doc["battery_divider"] = runtime_battery_divider;
  doc["temp_scale"] = runtime_temp_scale;
  doc["oil_scale"] = runtime_pressure_scale;
  doc["hyd_scale"] = runtime_hyd_pressure_scale;
  doc["fuel_empty"] = runtime_fuel_empty;
  doc["fuel_full"] = runtime_fuel_full;

  doc["battery_calculated"] = readBatteryVoltage();
  doc["temperature_calculated"] = readEngineTemp();
  doc["oil_calculated"] = readOilPressure();
  doc["hydraulic_calculated"] = readHydraulicPressure();
  doc["fuel_calculated"] = readFuelLevel();

  String out;
  serializeJson(doc, out);
  request->send(200, "application/json", out);
}

// ---------------------------------------------------------------------------
// GET /api/backup  and  POST /api/restore  (full config download / upload)
// ---------------------------------------------------------------------------
static void handleBackup(AsyncWebServerRequest* request) {
  const BobcatSettings& s = g_settingsManager.getSettings();
  DynamicJsonDocument doc(3072);
  doc["version"] = SETTINGS_VERSION;
  JsonObject eng = doc.createNestedObject("engine");
  eng["glow"] = s.glowPlugDuration / 1000;
  eng["crank"] = s.crankingTimeout / 1000;
  eng["cooldown"] = s.cooldownDuration / 1000;
  eng["glowAssist"] = s.glowAssistDuration / 1000;
  JsonObject th = doc.createNestedObject("thresholds");
  th["maxTemp"] = s.maxCoolantTemp;
  th["minOil"] = s.minOilPressure;
  th["minHyd"] = s.minHydPressure;
  th["minV"] = s.minBatteryVoltage;
  th["maxV"] = s.maxBatteryVoltage;
  JsonObject cal = doc.createNestedObject("cal");
  cal["tempScale"] = s.tempSensorScale;
  cal["oilScale"] = s.oilPressureScale;
  cal["hydScale"] = s.hydPressureScale;
  cal["batteryDivider"] = s.batteryDivider;
  cal["fuelEmpty"] = s.fuelLevelEmpty;
  cal["fuelFull"] = s.fuelLevelFull;
  cal["fuelLow"] = s.fuelLevelLowThreshold;
  JsonObject mq = doc.createNestedObject("mqtt");
  mq["enabled"] = (bool)s.mqttEnabled;
  mq["host"] = s.mqttHost;
  mq["port"] = s.mqttPort;
  mq["topic"] = s.mqttTopic;
  JsonObject tm = doc.createNestedObject("time");
  tm["ntp"] = (bool)s.ntpEnabled;
  tm["utcOffset"] = s.utcOffset;
  JsonObject wf = doc.createNestedObject("wifi");
  wf["ssid"] = s.wifiSSID;
  wf["ap"] = s.apSSID;
  JsonArray pins = doc.createNestedArray("pins");
  for (int i = 0; i < PIN_FUNC_COUNT; i++) {
    JsonObject p = pins.createNestedObject();
    p["func"] = PIN_TABLE[i].func;
    p["gpio"] = g_settingsManager.getPinGpio(i);
  }
  AsyncResponseStream* resp = request->beginResponseStream("application/json");
  resp->addHeader("Content-Disposition", "attachment; filename=\"bobcat743-config.json\"");
  serializeJson(doc, *resp);
  request->send(resp);
}

static void onRestoreBody(AsyncWebServerRequest* request, uint8_t* data, size_t len,
                          size_t index, size_t total) {
  DynamicJsonDocument doc(3072);
  if (deserializeJson(doc, data, len)) { sendErr(request, 400, "invalid JSON"); return; }
  const BobcatSettings& c = g_settingsManager.getSettings();
  int applied = 0;

  JsonObject eng = doc["engine"];
  if (!eng.isNull()) {
    g_settingsManager.updateEngineSettings((uint32_t)eng["glow"] * 1000,
                                           (uint32_t)eng["crank"] * 1000,
                                           (uint32_t)eng["cooldown"] * 1000);
    if (eng.containsKey("glowAssist"))
      g_settingsManager.setGlowAssist((uint32_t)eng["glowAssist"] * 1000);
    applied++;
  }
  JsonObject th = doc["thresholds"];
  if (!th.isNull()) {
    g_settingsManager.updateAlarmThresholds((int16_t)th["maxTemp"], (int16_t)th["minOil"],
                                            (float)th["minV"], (float)th["maxV"]);
    g_settingsManager.updateHydraulicThreshold((int16_t)th["minHyd"]);
    applied++;
  }
  JsonObject cal = doc["cal"];
  if (!cal.isNull()) {
    g_settingsManager.updateCalibration((float)cal["tempScale"], (float)cal["oilScale"],
                                        (float)cal["hydScale"], (float)cal["batteryDivider"],
                                        (uint16_t)cal["fuelEmpty"], (uint16_t)cal["fuelFull"],
                                        (uint8_t)cal["fuelLow"]);
    applied++;
  }
  JsonObject mq = doc["mqtt"];
  if (!mq.isNull()) {
    g_settingsManager.updateMqtt(mq["enabled"] ? 1 : 0, mq["host"] | "",
                                 (uint16_t)(mq["port"] | 1883), mq["topic"] | "");
    applied++;
  }
  JsonObject tm = doc["time"];
  if (!tm.isNull()) {
    g_settingsManager.updateTime(tm["ntp"] ? 1 : 0, (int32_t)(tm["utcOffset"] | 0));
    applied++;
  }
  JsonObject wf = doc["wifi"];
  if (!wf.isNull()) {
    if (wf.containsKey("ap"))   g_settingsManager.updateApSettings(wf["ap"].as<const char*>(), c.apPassword);
    if (wf.containsKey("ssid")) g_settingsManager.updateWifiSettings(wf["ssid"].as<const char*>(), c.wifiPassword);
    applied++;
  }
  JsonArray pins = doc["pins"];
  if (!pins.isNull()) {
    for (JsonObject p : pins) {
      int idx = pinFuncIndex(p["func"] | "");
      if (idx >= 0) { String e; g_settingsManager.updatePinMap(idx, (uint8_t)(int)p["gpio"], e); }
    }
    applied++;
  }

  g_systemState.configDirty = true;
  g_systemState.reloadCalibration = true;
  StaticJsonDocument<96> r;
  r["ok"] = true;
  r["applied"] = applied;
  String out;
  serializeJson(r, out);
  request->send(200, "application/json", out);
}

// Serve a LittleFS file with no-store so UI updates always take effect (no stale cache).
static void sendStatic(AsyncWebServerRequest* r, const char* path, const char* type) {
  AsyncWebServerResponse* resp = r->beginResponse(LittleFS, path, type);
  resp->addHeader("Cache-Control", "no-store");
  r->send(resp);
}

// ---------------------------------------------------------------------------
// Server setup
// ---------------------------------------------------------------------------
void setupWebServer() {
  if (!LittleFS.begin()) {
    Serial.println("WARNING: LittleFS mount failed - static files unavailable, API still up");
  }

  const BobcatSettings& s = g_settingsManager.getSettings();

  WiFi.mode(WIFI_AP_STA);

  // Station: only if an SSID is configured (no hard-coded credentials).
  if (strlen(s.wifiSSID) > 0) {
    Serial.printf("Connecting to station '%s'...\n", s.wifiSSID);
    WiFi.begin(s.wifiSSID, s.wifiPassword);
    int t = 0;
    while (WiFi.status() != WL_CONNECTED && t < 20) { delay(500); Serial.print("."); t++; }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("Station connected, IP: "); Serial.println(WiFi.localIP());
    } else {
      Serial.println("Station connect failed - AP only");
    }
  } else {
    Serial.println("No station SSID configured - AP only");
  }

  // Access point (always on). Open AP only if password too short to be valid.
  if (strlen(s.apPassword) >= 8) {
    WiFi.softAP(s.apSSID, s.apPassword);
  } else {
    WiFi.softAP(s.apSSID);
  }
  Serial.print("AP '"); Serial.print(s.apSSID); Serial.print("' IP: ");
  Serial.println(WiFi.softAPIP());

  // ---- Captive portal helpers ----
  server.onNotFound([](AsyncWebServerRequest* request) {
    request->redirect("http://192.168.4.1/");
  });
  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest* r){ r->redirect("http://192.168.4.1/"); });
  server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest* r){ r->redirect("http://192.168.4.1/"); });

  // ---- Static files ----
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* r){ sendStatic(r, "/index.html", "text/html"); });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest* r){ sendStatic(r, "/style.css", "text/css"); });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest* r){ sendStatic(r, "/script.js", "text/javascript"); });
  server.on("/logo.png", HTTP_GET, [](AsyncWebServerRequest* r){ r->send(LittleFS, "/logo.png", "image/png"); });
  server.on("/settings.html", HTTP_GET, [](AsyncWebServerRequest* r){ sendStatic(r, "/settings.html", "text/html"); });
  server.on("/settings.js", HTTP_GET, [](AsyncWebServerRequest* r){ sendStatic(r, "/settings.js", "text/javascript"); });

  // ---- New /api contract ----
  server.on("/api/status", HTTP_GET, handleApiStatus);
  server.on("/api/control", HTTP_POST, [](AsyncWebServerRequest* r){}, NULL, onControlBody);
  server.on("/api/settings", HTTP_GET, handleGetSettings);
  server.on("/api/settings", HTTP_POST, [](AsyncWebServerRequest* r){}, NULL, onSettingsBody);
  server.on("/api/backup", HTTP_GET, handleBackup);
  server.on("/api/restore", HTTP_POST, [](AsyncWebServerRequest* r){}, NULL, onRestoreBody);
  server.on("/api/pins", HTTP_GET, handleGetPins);
  server.on("/api/pins", HTTP_POST, [](AsyncWebServerRequest* r){}, NULL, onPinsBody);

  // ---- Legacy / diagnostics ----
  server.on("/status", HTTP_GET, handleLegacyStatus);
  server.on("/api/raw-sensors", HTTP_GET, handleRawSensors);

  server.on("/api/reset-calibration", HTTP_POST, [](AsyncWebServerRequest* request){
    g_settingsManager.resetCalibrationDefaults();
    g_systemState.configDirty = true;
    g_systemState.reloadCalibration = true;
    request->send(200, "application/json", "{\"ok\":true,\"message\":\"calibration reset\"}");
  });

  server.on("/api/factory-reset", HTTP_POST, [](AsyncWebServerRequest* request){
    // Defer the actual erase + restart to loop() (no flash writes in handler).
    g_systemState.factoryResetRequested = true;
    request->send(200, "application/json", "{\"ok\":true,\"message\":\"factory reset scheduled\"}");
  });

  // Legacy override endpoint is intentionally NOT a bare crank anymore.
  server.on("/override", HTTP_GET, [](AsyncWebServerRequest* request){
    sendErr(request, 405, "override must be POSTed via /api/control {\"action\":\"override\"}");
  });

  ElegantOTA.begin(&server);
  server.begin();
  Serial.println("Web server started");
}
