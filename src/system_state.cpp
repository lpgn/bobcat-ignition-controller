/*
 * System State Management Implementation for Bobcat Ignition Controller
 *
 * Ignition key sequence: OFF -> ON -> GLOW -> START (momentary) -> RUNNING.
 * Safety interlocks + battery-OK are enforced at the single crank choke point
 * (attemptCrank) so NO path can engage the starter without them.
 */

#include "system_state.h"
#include "config.h"
#include "hardware.h"
#include "safety.h"
#include "settings.h"

SystemState_t g_systemState = {};

// Engine temp at/above which a start skips the glow-preheat wait (warm start).
static const float WARM_ENGINE_TEMP_C = 50.0f;

// Enter the START state and energize glow. Glow is ALWAYS safe on a start attempt
// (it only preheats). The STARTER itself is gated every loop inside the START case
// (interlocks + battery), so holding START always glows even when the starter is
// blocked, and the starter engages the instant the interlocks are satisfied.
static void beginStart(unsigned long now) {
  controlMainPower(true);
  // Carry over an existing glow preheat (from the GLOW step) so you don't wait
  // twice; only (re)start the glow timer if we weren't already glowing.
  if (!outputOn(PIN_GLOW)) g_systemState.glowPlugStartTime = now;
  controlGlowPlugs(true);
  g_systemState.currentState = START;
  g_systemState.ignitionStartTime = 0;    // starter engages later, once glow has warmed the motor
  g_systemState.startHoldTime = now;
  g_systemState.oilOkSince = 0;
  // Latch "warm" ONCE (no per-loop flicker). Ignore implausible temps (>120C or
  // <WARM) so a floating/disconnected sensor never triggers a warm skip.
  float t0 = readEngineTemp();
  g_systemState.warmStart = (t0 >= WARM_ENGINE_TEMP_C && t0 <= 120.0f);
  Serial.println(g_systemState.warmStart ? "START: warm engine - will crank immediately"
                                         : "START: glow preheating, then crank");
}

void runIgnitionSequence() {
  unsigned long now = millis();
  const BobcatSettings& s = g_settingsManager.getSettings();
  uint32_t glowDur = s.glowPlugDuration;
  uint32_t crankTimeout = s.crankingTimeout;
  int16_t minOil = s.minOilPressure;

  // 1. Forced OFF (master power off) whenever key is OFF.
  if (g_systemState.keyPosition == 0 && g_systemState.currentState != OFF) {
    Serial.println("KEY OFF - system shutdown");
    g_systemState.currentState = OFF;
    controlMainPower(false);
    controlGlowPlugs(false);
    controlStarter(false);
    controlLights(false);
    controlBuzzer(false);
    g_systemState.workLightsOn = false;
    g_systemState.keyStartHeld = false;
    g_systemState.glowPlugStartTime = 0;
    g_systemState.oilOkSince = 0;
  }

  // 2. Emergency stop: cut engine processes, keep main power (lights still work).
  //    NOTE: a diesel is stopped by the fuel lever - we never claim to stop it.
  if (g_systemState.emergencyStopPressed) {
    Serial.println("EMERGENCY STOP - cutting starter + glow, keeping power");
    controlStarter(false);
    controlGlowPlugs(false);
    g_systemState.keyStartHeld = false;
    g_systemState.keyPosition = 1;
    g_systemState.currentState = ON;
    g_systemState.glowPlugStartTime = 0;
    g_systemState.oilOkSince = 0;
    g_systemState.emergencyStopPressed = false;
  }

  // 3. Lights toggle (independent of engine state).
  if (g_systemState.lightsTogglePressed) {
    g_systemState.workLightsOn = !g_systemState.workLightsOn;
    controlLights(g_systemState.workLightsOn);
    Serial.println(g_systemState.workLightsOn ? "Work lights ON" : "Work lights OFF");
    g_systemState.lightsTogglePressed = false;
  }

  // 4. Per-state logic.
  switch (g_systemState.currentState) {
    case OFF:
      if (g_systemState.keyPosition >= 1) {
        g_systemState.currentState = ON;
        controlMainPower(true);
        Serial.println("KEY ON - system energized");
      }
      break;

    case ON:
      // Turn glow off if a previous glow cycle has expired.
      if (g_systemState.glowPlugStartTime > 0 && now - g_systemState.glowPlugStartTime >= glowDur) {
        controlGlowPlugs(false);
      }
      if (g_systemState.keyPosition == 2) {
        g_systemState.currentState = GLOW_PLUG;
        g_systemState.glowPlugStartTime = now;
        controlGlowPlugs(true);
        Serial.println("GLOW position - preheating");
      } else if (g_systemState.keyPosition >= 3 || g_systemState.keyStartHeld) {
        beginStart(now);   // glow on; starter gated in START
      }
      break;

    case GLOW_PLUG:
      if (g_systemState.keyPosition == 1) {
        g_systemState.currentState = ON;  // pause; keep glow running
      } else if (g_systemState.keyPosition >= 3 || g_systemState.keyStartHeld) {
        beginStart(now);   // glow on; starter gated in START
      } else if (now - g_systemState.glowPlugStartTime >= glowDur) {
        controlGlowPlugs(false);
        g_systemState.currentState = ON;
        g_systemState.keyPosition = 1;
        Serial.println("GLOW complete - returning to ON");
      }
      break;

    case START: {
      bool held = g_systemState.keyStartHeld && g_systemState.keyPosition >= 3;

      // Released -> stop everything, back to ON.
      if (!held) {
        controlStarter(false);
        controlGlowPlugs(false);
        g_systemState.currentState = ON;
        g_systemState.keyPosition = 1;
        Serial.println("Start released - returning to ON");
        break;
      }

      // Glow stays ON the whole hold: through the preheat wait AND the cranking.
      controlGlowPlugs(true);

      // Wait for glow to warm the motor before cranking - UNLESS the engine is
      // already warm (warm restart cranks immediately). glowPlugStartTime carried
      // over from an earlier GLOW-step preheat by beginStart().
      unsigned long glowElapsed = now - g_systemState.glowPlugStartTime;
      bool preheatDone = g_systemState.warmStart || glowElapsed >= glowDur;
      bool interlockOk = g_systemState.maintenanceMode ||
                         (readSeatBarSafety() && readNeutralSafety() &&
                          readBatteryVoltage() >= s.minBatteryVoltage);

      bool cranking = false;
      if (preheatDone && interlockOk) {
        if (g_systemState.ignitionStartTime == 0) {
          g_systemState.ignitionStartTime = now;   // starter engages now (crank-timeout starts here)
          Serial.println(g_systemState.warmStart ? "Warm engine - cranking immediately"
                                                  : "Glow preheat complete - cranking");
        }
        cranking = (now - g_systemState.ignitionStartTime) < crankTimeout;
      }
      controlStarter(cranking);

      // RUNNING detection: oil pressure OK sustained while actually cranking.
      if (cranking && readOilPressure() >= minOil) {
        if (g_systemState.oilOkSince == 0) {
          g_systemState.oilOkSince = now;
        } else if (now - g_systemState.oilOkSince >= RUNNING_OIL_CONFIRM_MS) {
          Serial.println("Engine RUNNING (oil confirmed) - releasing");
          controlStarter(false);
          controlGlowPlugs(false);
          g_systemState.currentState = RUNNING;
          g_systemState.keyPosition = 1;
          g_systemState.keyStartHeld = false;
          break;
        }
      } else {
        g_systemState.oilOkSince = 0;
      }

      // Crank window ended (starter engaged but timed out): cut starter + glow,
      // wait for the key to be released.
      if (g_systemState.ignitionStartTime != 0 &&
          (now - g_systemState.ignitionStartTime) >= crankTimeout) {
        controlStarter(false);
        controlGlowPlugs(false);
        Serial.println("Crank timeout - release + re-hold to retry");
      }
      break;
    }

    case RUNNING:
      if (g_systemState.keyPosition >= 3 || g_systemState.keyStartHeld) {
        Serial.println("HOT RESTART requested");
        beginStart(now);
      }
      // Vitals monitored by checkEngineVitals() from loop().
      break;

    case LOW_OIL_PRESSURE:
    case HIGH_TEMPERATURE:
      // Alert sub-states of RUNNING. Recovery handled by checkEngineVitals().
      // A plain key crank is refused here - deliberate override is required.
      if (g_systemState.keyStartHeld || g_systemState.keyPosition >= 3) {
        g_systemState.keyStartHeld = false;
        g_systemState.keyPosition = 1;
        Serial.println("Crank refused during alert - use override");
      }
      break;

    case ERROR:
      if (g_systemState.keyStartHeld || g_systemState.keyPosition >= 3) {
        g_systemState.keyStartHeld = false;
        g_systemState.keyPosition = 1;
      }
      break;
  }

  // Status LED reflects powered state; buzzer sounds during alert states.
  controlStatusLed(g_systemState.currentState != OFF);
  bool alert = (g_systemState.currentState == LOW_OIL_PRESSURE ||
                g_systemState.currentState == HIGH_TEMPERATURE ||
                g_systemState.currentState == ERROR);
  controlBuzzer(alert);
}

void updateEngineHours(unsigned long now) {
  static unsigned long lastTick = 0;
  if (lastTick == 0) { lastTick = now; return; }
  unsigned long dt = now - lastTick;
  lastTick = now;

  if (isEngineRunning()) {
    g_systemState.engineHoursAccumMs += dt;
  }
  // Fold accumulated running time into NVS every 60 s of runtime.
  if (g_systemState.engineHoursAccumMs >= 60000UL) {
    g_settingsManager.addEngineHours(g_systemState.engineHoursAccumMs / 3600000.0f);
    g_systemState.engineHoursAccumMs = 0;
    g_systemState.configDirty = true;   // loop() persists
  }
}

const char* systemStateToString(int state) {
  switch (state) {
    case OFF: return "OFF";
    case ON: return "ON";
    case GLOW_PLUG: return "GLOW_PLUG";
    case START: return "START";
    case RUNNING: return "RUNNING";
    case LOW_OIL_PRESSURE: return "LOW_OIL_PRESSURE";
    case HIGH_TEMPERATURE: return "HIGH_TEMPERATURE";
    case ERROR: return "ERROR";
    default: return "UNKNOWN";
  }
}

const char* apiStateName(int state) {
  switch (state) {
    case OFF: return "OFF";
    case ON: return "ON";
    case GLOW_PLUG: return "GLOW";
    case START: return "START";
    case RUNNING: return "RUNNING";
    case LOW_OIL_PRESSURE: return "LOW_OIL";
    case HIGH_TEMPERATURE: return "HIGH_TEMP";
    case ERROR: return "ERROR";
    default: return "OFF";
  }
}

int apiStateSeq(int state) {
  switch (state) {
    case OFF: return 0;
    case ON: return 1;
    case GLOW_PLUG: return 2;
    case START: return 3;
    case RUNNING:
    case LOW_OIL_PRESSURE:
    case HIGH_TEMPERATURE: return 4;
    case ERROR: return 1;
    default: return 0;
  }
}
