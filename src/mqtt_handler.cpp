/*
 * MQTT Telemetry Handler - implementation.
 *
 * READ-ONLY: publishes engine health/diagnostics and Home Assistant discovery.
 * It NEVER subscribes to a command topic and NEVER creates a HA switch/button/
 * number entity, so there is no path from MQTT back to any relay or ignition.
 *
 * ALL PubSubClient calls below run in loopTask (mqttLoop). Web handlers only
 * flip s_reconnectRequested (mqttRequestReconnect) and read s_connectedCached
 * (mqttConnected) - never the client itself.
 */

#include "mqtt_handler.h"
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <math.h>
#include "settings.h"
#include "hardware.h"
#include "system_state.h"
#include "config.h"

// ---------------------------------------------------------------------------
// Client + state (touched only from loopTask, except the two volatiles)
// ---------------------------------------------------------------------------
static WiFiClient   s_wifiClient;
static PubSubClient s_mqtt(s_wifiClient);

static volatile bool s_reconnectRequested = false;  // set by async handler
static volatile bool s_connectedCached    = false;  // read by async handler

static bool          s_discoverySent      = false;
static unsigned long s_lastConnectAttempt = 0;
static unsigned long s_lastPublish        = 0;

// Config snapshot (copied from settings in loopTask only).
static uint8_t  s_enabled = 0;
static char     s_host[64] = {0};
static uint16_t s_port     = 1883;
static char     s_baseTopic[64] = {0};

static const unsigned long RECONNECT_INTERVAL_MS = 5000;
static const unsigned long PUBLISH_INTERVAL_MS   = 10000;

// Stable HA node/unique-id prefix (independent of the configurable base topic).
static const char* HA_NODE = "bobcat743";

// ---------------------------------------------------------------------------
// Config snapshot
// ---------------------------------------------------------------------------
static void loadMqttConfig() {
  const BobcatSettings& s = g_settingsManager.getSettings();
  s_enabled = s.mqttEnabled;
  strncpy(s_host, s.mqttHost, sizeof(s_host) - 1);
  s_host[sizeof(s_host) - 1] = '\0';
  s_port = s.mqttPort ? s.mqttPort : 1883;
  strncpy(s_baseTopic, s.mqttTopic, sizeof(s_baseTopic) - 1);
  s_baseTopic[sizeof(s_baseTopic) - 1] = '\0';
  if (s_baseTopic[0] == '\0') strcpy(s_baseTopic, "bobcat743");
}

// ---------------------------------------------------------------------------
// Discovery tables (SENSORS + BINARY_SENSORS only - no controllable entities)
// ---------------------------------------------------------------------------
struct SensorDef {
  const char* field;
  const char* name;
  const char* devClass;    // nullptr => omit
  const char* unit;        // nullptr => omit
  const char* stateClass;  // nullptr => omit
};

static const SensorDef SENSORS[] = {
  { "state",       "State",               nullptr,       nullptr, nullptr },
  { "engineHours", "Engine Hours",        nullptr,       "h",     "total_increasing" },
  { "engineTemp",  "Engine Temperature",  "temperature", "\xC2\xB0""C", "measurement" },
  { "oilPressure", "Oil Pressure",        "pressure",    "psi",   "measurement" },
  { "hydraulic",   "Hydraulic Pressure",  "pressure",    "psi",   "measurement" },
  { "battery",     "Battery Voltage",     "voltage",     "V",     "measurement" },
  { "fuel",        "Fuel Level",          nullptr,       "%",     "measurement" },
};

struct BinaryDef {
  const char* field;
  const char* name;
  const char* devClass;
};

static const BinaryDef BINARIES[] = {
  { "power",        "Main Power",         "power" },
  { "glow",         "Glow Plugs",         "power" },
  { "starter",      "Starter",            "power" },
  { "lights",       "Work Lights",        "power" },
  { "faultOil",     "Oil Pressure Fault", "problem" },
  { "faultTemp",    "Over Temperature",   "problem" },
  { "faultBattery", "Battery Fault",      "problem" },
};

// Shared HA device block so all entities group under one device.
static void addDeviceBlock(JsonObject dev) {
  JsonArray ids = dev.createNestedArray("identifiers");
  ids.add(HA_NODE);
  dev["name"] = "Bobcat 743";
  dev["model"] = "743 / Kubota V1702";
  dev["manufacturer"] = "Bobcat";
}

// ---------------------------------------------------------------------------
// Discovery + telemetry publishing (loopTask only)
// ---------------------------------------------------------------------------
static void publishDiscovery() {
  char stateTopic[96];
  char availTopic[96];
  snprintf(stateTopic, sizeof(stateTopic), "%s/state", s_baseTopic);
  snprintf(availTopic, sizeof(availTopic), "%s/availability", s_baseTopic);

  // Sensors
  for (const SensorDef& d : SENSORS) {
    StaticJsonDocument<640> doc;
    char uid[48];
    snprintf(uid, sizeof(uid), "%s_%s", HA_NODE, d.field);
    doc["name"] = d.name;
    doc["unique_id"] = uid;
    doc["object_id"] = uid;
    doc["state_topic"] = stateTopic;
    doc["availability_topic"] = availTopic;
    doc["payload_available"] = "online";
    doc["payload_not_available"] = "offline";
    char vt[48];
    snprintf(vt, sizeof(vt), "{{ value_json.%s }}", d.field);
    doc["value_template"] = vt;
    if (d.devClass)   doc["device_class"] = d.devClass;
    if (d.unit)       doc["unit_of_measurement"] = d.unit;
    if (d.stateClass) doc["state_class"] = d.stateClass;
    addDeviceBlock(doc.createNestedObject("device"));

    char cfgTopic[96];
    snprintf(cfgTopic, sizeof(cfgTopic), "homeassistant/sensor/%s_%s/config", HA_NODE, d.field);
    char payload[640];
    size_t n = serializeJson(doc, payload, sizeof(payload));
    s_mqtt.publish(cfgTopic, (const uint8_t*)payload, n, true);  // retained
  }

  // Binary sensors (booleans -> ON/OFF)
  for (const BinaryDef& d : BINARIES) {
    StaticJsonDocument<640> doc;
    char uid[48];
    snprintf(uid, sizeof(uid), "%s_%s", HA_NODE, d.field);
    doc["name"] = d.name;
    doc["unique_id"] = uid;
    doc["object_id"] = uid;
    doc["state_topic"] = stateTopic;
    doc["availability_topic"] = availTopic;
    doc["payload_available"] = "online";
    doc["payload_not_available"] = "offline";
    char vt[64];
    snprintf(vt, sizeof(vt), "{{ 'ON' if value_json.%s else 'OFF' }}", d.field);
    doc["value_template"] = vt;
    doc["payload_on"] = "ON";
    doc["payload_off"] = "OFF";
    if (d.devClass) doc["device_class"] = d.devClass;
    addDeviceBlock(doc.createNestedObject("device"));

    char cfgTopic[96];
    snprintf(cfgTopic, sizeof(cfgTopic), "homeassistant/binary_sensor/%s_%s/config", HA_NODE, d.field);
    char payload[640];
    size_t n = serializeJson(doc, payload, sizeof(payload));
    s_mqtt.publish(cfgTopic, (const uint8_t*)payload, n, true);  // retained
  }
}

static void publishState() {
  const BobcatSettings& s = g_settingsManager.getSettings();

  float temp = readEngineTemp();
  float oil  = readOilPressure();
  float hyd  = readHydraulicPressure();
  float batt = readBatteryVoltage();
  float fuel = readFuelLevel();
  bool  running = isEngineRunning();

  StaticJsonDocument<384> doc;
  doc["state"] = apiStateName(g_systemState.currentState);
  doc["engineHours"] = g_settingsManager.getEngineHours() +
                       g_systemState.engineHoursAccumMs / 3600000.0;
  doc["engineTemp"] = (int)lroundf(temp);
  doc["oilPressure"] = (int)lroundf(oil);
  doc["hydraulic"] = (int)lroundf(hyd);
  doc["battery"] = lroundf(batt * 10.0f) / 10.0;
  doc["fuel"] = (int)lroundf(fuel);
  doc["power"]   = outputOn(PIN_MAIN_POWER);
  doc["glow"]    = outputOn(PIN_GLOW);
  doc["starter"] = outputOn(PIN_STARTER);
  doc["lights"]  = outputOn(PIN_LIGHTS);
  doc["faultOil"]     = running && (oil < s.minOilPressure);
  doc["faultTemp"]    = temp > s.maxCoolantTemp;
  doc["faultBattery"] = batt < s.minBatteryVoltage;

  char stateTopic[96];
  snprintf(stateTopic, sizeof(stateTopic), "%s/state", s_baseTopic);
  char payload[384];
  size_t n = serializeJson(doc, payload, sizeof(payload));
  s_mqtt.publish(stateTopic, (const uint8_t*)payload, n, false);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
void mqttInit() {
  loadMqttConfig();
  s_mqtt.setBufferSize(1024);   // room for retained discovery payloads
  s_discoverySent = false;
  s_connectedCached = false;
  // No blocking connect here; mqttLoop() connects when WiFi + config are ready.
}

void mqttRequestReconnect() {
  // Async-handler-safe: just flip the flag. loopTask does the real work.
  s_reconnectRequested = true;
}

bool mqttConnected() {
  // Cached bool only - never touches the client from the caller's task.
  return s_connectedCached;
}

void mqttLoop() {
  // ---- Consume a settings-change reconnect request ----
  if (s_reconnectRequested) {
    s_reconnectRequested = false;
    if (s_mqtt.connected()) s_mqtt.disconnect();
    loadMqttConfig();
    s_discoverySent = false;
    s_lastConnectAttempt = 0;   // allow an immediate reconnect attempt
  }

  // ---- Disabled: ensure disconnected ----
  if (!s_enabled) {
    if (s_mqtt.connected()) s_mqtt.disconnect();
    s_connectedCached = false;
    return;
  }

  // ---- No WiFi yet: retry later ----
  if (WiFi.status() != WL_CONNECTED) {
    s_connectedCached = false;
    return;
  }

  // ---- Non-blocking reconnect with backoff ----
  if (!s_mqtt.connected()) {
    unsigned long now = millis();
    if (s_lastConnectAttempt != 0 && (now - s_lastConnectAttempt) < RECONNECT_INTERVAL_MS) {
      s_connectedCached = false;
      return;
    }
    s_lastConnectAttempt = now;
    if (strlen(s_host) == 0) { s_connectedCached = false; return; }

    s_mqtt.setServer(s_host, s_port);

    char availTopic[96];
    snprintf(availTopic, sizeof(availTopic), "%s/availability", s_baseTopic);

    String clientId = String(HA_NODE) + "-" + WiFi.macAddress();

    // LWT: broker publishes "offline" (retained) on availTopic if we drop.
    bool connected = s_mqtt.connect(clientId.c_str(), NULL, NULL,
                                    availTopic, 0, true, "offline");
    if (connected) {
      s_mqtt.publish(availTopic, "online", true);   // retained availability
      if (!s_discoverySent) {
        publishDiscovery();
        s_discoverySent = true;
      }
      publishState();
      s_lastPublish = millis();
    }
    s_connectedCached = s_mqtt.connected();
    return;
  }

  // ---- Connected: service the client + periodic telemetry ----
  s_mqtt.loop();
  unsigned long now = millis();
  if ((now - s_lastPublish) >= PUBLISH_INTERVAL_MS) {
    s_lastPublish = now;
    publishState();
  }
  s_connectedCached = s_mqtt.connected();
}
