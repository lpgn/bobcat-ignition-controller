# Hardware

Board: LILYGO T-Relay ESP32 (4 relays)

## Relay Outputs (Active HIGH)

- MAIN_POWER (Relay 1) → GPIO21
- GLOW_PLUGS (Relay 2) → GPIO19
- STARTER (Relay 3) → GPIO18
- LIGHTS (Relay 4) → GPIO5

Power connections:

- Battery → main fuse → MAIN_POWER relay → distribution
- Glow plugs: GLOW_PLUGS relay → bus bar
- Starter: STARTER relay → starter solenoid (use diode across solenoid)
- Lights: LIGHTS relay → lamps

## Analog Inputs (ESP32 ADC1, 12-bit 0–4095, 0–3.3V)

### Battery Voltage (GPIO36 / ADC1_CH0)

- Divider for 12V systems: R1 = 100kΩ (from battery to ADC), R2 = 33kΩ (ADC to GND)
- Expected scale: Vadc ≈ Vbat × (R2 / (R1+R2)) ≈ Vbat × 0.248
- Series 1kΩ to ADC for protection (optional but recommended)

### Engine Temp NTC (GPIO39 / ADC1_CH3)

- 10kΩ NTC to GND, 10kΩ pull-up to 3.3V, midpoint to ADC39
- Beta ~3950 type typical; calibration handled in firmware

### Oil Pressure Sender (GPIO34 / ADC1_CH6)

- 0–180Ω resistive sender: use 220Ω pull-up to 3.3V; midpoint to ADC34
- Adjust scale in firmware as needed; keep current < 15mA

### Fuel Level Sender (GPIO35 / ADC1_CH7)

- Float sender 240-33Ω: use 330Ω pull-up to 3.3V; midpoint to ADC35
- Standard automotive: 240Ω (empty) to 33Ω (full tank)
- Calibrate empty/full via API; values stored in NVS

### Hydraulic Pressure Sender (GPIO33 / ADC1_CH5)

- Resistive sender 0–180Ω: use 220Ω pull-up to 3.3V; midpoint to ADC33
- Similar to oil pressure sensor; adjust scale in firmware as needed

## Digital Inputs

### Status Inputs

- Alternator Charge → GPIO22 (internal pull-up enabled, active-low)
- Engine Run Feedback → GPIO26 (internal pull-up enabled, active-low)

### Power Management

- Wake Up Button → GPIO0 (BOOT button)
- Sleep Enable → GPIO12

## 5V Sensor Compatibility

- ESP32 ADC maximum input: 3.3V (exceeding this damages the chip)
- For any 5V sensors: use 2:1 voltage divider (e.g., 10kΩ + 10kΩ to GND)
- Current design uses resistive senders with 3.3V pull-ups (no 5V divider needed)

## Grounding & Shielding

- Star ground at battery negative; tie sensor returns near ADC ground reference
- Keep ADC wires short; twist with ground where possible
- Use automotive fuses and proper gauge wiring

## Notes

- Relays default OFF at boot; Emergency stop drops all relays
- Refer to schematics.mmd for the complete wiring diagram
