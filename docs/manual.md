# Bobcat Ignition Controller — Manual

This system emulates an ignition key for a Bobcat 743 (Kubota V1702-BA). It controls main power, glow plugs, starter, lights, and shows safety alerts.

## Use

- OFF → ON: Powers electronics and sensors.
- GLOW: 20s preheat. Wait for countdown to finish.
- START: Hold to crank (max 5s). Release to return to GLOW.
- RUNNING: Monitoring active; alerts shown in UI. Engine stop is manual (fuel lever).

Controls are available on the built-in web UI (access point and/or LAN). Lights toggle is independent.

## Safety

- Low battery lockout (<10.5V) prevents starting.
- Starter auto-timeout (5s max).
- Oil pressure and temperature are alerts only; no auto-shutdown.
- Emergency stop: immediately drops all relays.

## Updates and files

- Web UI assets are served from LittleFS.
- For build and OTA steps, see .github/copilot-instructions.md (single source of truth).

## Troubleshooting

- If UI is unreachable after firmware update, wait ~15–20s and refresh.
- Use the AP at 192.168.4.1 if not on LAN.
