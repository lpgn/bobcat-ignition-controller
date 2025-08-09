# Hardware

- Board: LILYGO T-Relay ESP32 (4 relays)

Relays (Active HIGH):

- MAIN_POWER (Relay 1) → GPIO21
- GLOW_PLUGS (Relay 2) → GPIO19
- STARTER (Relay 3) → GPIO18
- LIGHTS (Relay 4) → GPIO5

Analog inputs (ESP32 ADC1, 12-bit 0–4095, 0–3.3V):

- Engine Temp (NTC) → GPIO39 (ADC1_CH3)
- Oil Pressure (resistive sender) → GPIO34 (ADC1_CH6)
- Battery Voltage (divider) → GPIO36 (ADC1_CH0)
- Fuel Level (float sender) → GPIO35 (ADC1_CH7)

Digital inputs:

- Alternator Charge → GPIO22
- Engine Run Feedback → GPIO26

Notes:

- Use automotive fuses and proper gauge wiring.
- Relays default OFF at boot; Emergency stop drops all relays.
- See wiring.md for exact resistor values and circuits, and schematics.mmd for the diagram.
