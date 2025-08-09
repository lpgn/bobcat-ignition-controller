# Wiring

Power/relays:

- Battery → main fuse → MAIN_POWER relay → distribution
- Glow plugs: GLOW_PLUGS relay → bus bar
- Starter: STARTER relay → starter solenoid (use diode across solenoid)
- Lights: LIGHTS relay → lamps

Battery voltage (GPIO36 / ADC1_CH0):

- Divider for 12V systems: R1 = 100k (from battery to ADC), R2 = 33k (ADC to GND)
- Expected scale: Vadc ≈ Vbat × (R2 / (R1+R2)) ≈ Vbat × 0.248
- Ensure a series 1k to ADC for protection is acceptable but not required.

Engine temp NTC (GPIO39 / ADC1_CH3):

- 10k NTC to GND, 10k pull-up to 3.3V, midpoint to ADC39
- Beta ~3950 type typical; calibration handled in firmware.

Oil pressure sender (GPIO34 / ADC1_CH6):

- 0–180Ω resistive sender: use a 220Ω pull-up to 3.3V; midpoint to ADC34
- Adjust scale in firmware as needed; keep current < 15mA.

Fuel level sender (GPIO35 / ADC1_CH7):

- Float sender 0–240Ω: use 330Ω pull-up to 3.3V; midpoint to ADC35
- Calibrate empty/full via API; values stored in NVS.

Grounding/shielding:

- Star ground at battery negative; tie sensor returns near ADC ground reference.
- Keep ADC wires short; twist with ground where possible.

Refer to schematics.mmd for the diagram.
