/*
 * MQTT Telemetry Handler for Bobcat Ignition Controller
 *
 * READ-ONLY health/diagnostics telemetry to an MQTT broker, with Home Assistant
 * MQTT-discovery so HA auto-creates sensor / binary_sensor entities. There are
 * intentionally NO command topics and NO control entities (switch/button/number):
 * nothing here can ever actuate a relay or the ignition.
 *
 * THREAD-SAFETY: PubSubClient is NOT thread-safe and its socket ops can block.
 * ALL client work (connect / loop / publish / disconnect) happens in loopTask
 * via mqttLoop(). Async web handlers must ONLY call mqttRequestReconnect()
 * (which sets a volatile flag) and mqttConnected() (which reads a cached bool).
 * They must never touch the PubSubClient instance directly.
 */

#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

// Construct the client over a WiFiClient. No blocking connect here.
// Call once in setup() after settings + WiFi are up.
void mqttInit();

// Drive the connection + publish telemetry. Call once per loop() (loopTask only).
void mqttLoop();

// Cached live-connection state for /api/status. Safe to call from async handlers.
bool mqttConnected();

// Async-handler-safe: request loopTask to disconnect, reload config and reconnect.
// Sets a volatile flag consumed by mqttLoop(); does NOT touch the client.
void mqttRequestReconnect();

#endif // MQTT_HANDLER_H
