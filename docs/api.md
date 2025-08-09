# REST API

Note: exact paths may vary; this describes the intended shape. All logic lives in C++.

## Status

- GET /status → { battery_v, temp_c, oil_kpa, fuel_pct, hyd_kpa, wifi: {...}, state: "RUNNING" }

## Raw sensors (debug)

- GET /api/raw-sensors → { battery_raw, temp_raw, oil_raw, fuel_raw, hyd_raw, ... }

## Settings

- GET /api/settings
- POST /api/settings { key, value } → validate, persist, apply

## Calibration

- POST /api/auto-calibrate (FormData: sensor, actual_value)
- POST /api/reset-calibration → clears prefs and reloads runtime constants
