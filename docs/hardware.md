# Hardware

- Board: LILYGO T-Relay ESP32 (4 relays)
- Critical relays (Active HIGH):
  - MAIN_POWER_PIN = 21
  - GLOW_PLUGS_PIN = 19
  - STARTER_PIN = 18
  - LIGHTS_PIN = 5
- Analog inputs: TEMP=39, OIL=34, BAT=36, FUEL=35
- Digital: ALT=22, RUN_FB=26

Notes:

- Use automotive fuses and proper gauge wiring.
- Relays default OFF at boot. Emergency stop drops all relays.
- See schematics.mmd for wiring overview.
