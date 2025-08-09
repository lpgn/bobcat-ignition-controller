# Documentation Index

Welcome to the Bobcat Ignition Controller documentation.

This safety‑critical ESP32 firmware controls ignition, glow plugs, starter, and lights for a Bobcat 743 (Kubota V1702-BA). All business logic is implemented in C++; the web UI is thin and only handles UI and HTTP requests.

## Quick links

- Getting started: [getting-started.md](getting-started.md)
- Build & Deploy (PlatformIO + ElegantOTA): [build-and-deploy.md](build-and-deploy.md)
- Calibration (sensors, rules): [calibration.md](calibration.md)
- Safety & constraints: [safety.md](safety.md)
- Architecture (state machine, modules): [architecture.md](architecture.md)
- REST API (status/settings/calibration): [api.md](api.md)
- Troubleshooting: [troubleshooting.md](troubleshooting.md)

## Hardware docs

- [hardware/hardware.md](hardware/hardware.md) (overview + wiring)
- [hardware/board_pinout.md](hardware/board_pinout.md) (LILYGO T-Relay pin reference)
- [hardware/gpio_connections.md](hardware/gpio_connections.md) (project GPIO mapping)
- [hardware/wiring_guide.md](hardware/wiring_guide.md) (installation)
- [hardware/sensor_wiring_guide.md](hardware/sensor_wiring_guide.md) (sensors)
- [schematics.mmd](schematics.mmd) (Mermaid diagram)

## Standards and principles

- Logic location: All calculations, calibrations, validation, and state management in C++. JavaScript only for UI, basic validation, and fetch().
- Never use delay(): use millis() patterns.
- Manual validation: ElegantOTA uploads must be done with a visible browser (no headless) for verification.
