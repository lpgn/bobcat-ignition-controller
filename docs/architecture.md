# Architecture

## Modules

- src/main.cpp: entry and loop (thin)
- src/system_state.cpp: state machine (OFF/ON/GLOW_PLUG/START/RUNNING)
- src/hardware.cpp: GPIO and sensor reads (uses runtime calibration)
- src/safety.cpp: safety checks (no auto-shutdown; alert-only)
- src/web_interface.cpp: REST endpoints and web assets
- src/settings.cpp: NVS-backed settings

## Data flow

- UI triggers fetch() → C++ endpoint → validate/compute → store in NVS → runtime vars updated → UI polls status endpoints

## Constraints

- No delay(); use millis()
- Business logic in C++
- Manual OTA via ElegantOTA (visible)
