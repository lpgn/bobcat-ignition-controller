/*
 * Web Interface Header for Bobcat Ignition Controller
 * Implements the /api contract consumed by the dashboard.
 *
 * IMPORTANT: handlers run in the async_tcp task. They ONLY read state or set
 * flags / mutate RAM settings. All flash writes, relay actuation and sleep are
 * performed by loop() (see system_state.h cross-task flags).
 */

#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <ESPAsyncWebServer.h>
#include "system_state.h"

void setupWebServer();

#endif // WEB_INTERFACE_H
