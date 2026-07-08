# Hardware

**🚨 SAFETY CRITICAL — READ [SAFETY_CRITICAL.md](SAFETY_CRITICAL.md) FIRST 🚨**

## Verified board facts

- **Board:** LilyGO T-Relay 4-Channel, ESP32-WROVER
- **Relays:** HRS4H-S-DC5V, **ACTIVE-HIGH**, **10 A max DC contacts**
- **Onboard relays:** K1 = GPIO21, K2 = GPIO19, K3 = GPIO18, K4 = GPIO5
- **Onboard LED (LedLink):** GPIO25
- **Header (spare) pins broken out:** 2, 4, 12, 13, 14, 15, 22, 23, 26, 27, 32, 33, 34, 35, 36, 39
- **ADC1 pins usable while Wi-Fi is active:** 32, 33, 34, 35, 36, 39
  - **Input-only (no internal pull-up):** 34, 35, 36, 39
- **Strapping pins to avoid for I/O:** 2, 4, 12, 15 (12 = MTDI flash-voltage strap; must be LOW at boot)

## Load assignment for the Bobcat 743

| Relay | GPIO | Function | Wiring requirement |
|-------|------|----------|-------------------|
| K1 | 21 | Main power relay | Safe within 10 A contact rating for electrical distribution |
| K2 | 19 | Glow plugs | **PILOT ONLY** — drive the factory high-current glow relay coil. Glow plugs draw 40–80 A; the 10 A T-Relay contact would weld. |
| K3 | 18 | Starter | **PILOT ONLY** — drive the starter solenoid coil (5–15 A inrush). **Mandatory 1N4007 flyback diode** across the coil. |
| K4 | 5 | Lights | **PILOT ONLY** — drive an external 30/40 A automotive relay. 2×55 W ≈ 9.2 A leaves no margin on a 10 A contact. |

### ⚠️ Relays are 10 A — always pilot external contactors/solenoids

The T-Relay board is a **logic / pilot controller**, not a high-current switch.
Never pass the Bobcat's high-current loads (glow plugs, starter, lights) directly through the small relay contacts.

## Analog sensors (ADC1 only)

While Wi-Fi is active, only ADC1 pins may be used for analog input.

| Sensor | GPIO | ADC1 channel | Notes |
|--------|------|--------------|-------|
| Engine temperature | 34 | ADC1_CH6 | Input-only; no internal pull-up. Use an external divider or NTC circuit. |
| Oil pressure | 35 | ADC1_CH7 | Input-only; no internal pull-up. |
| Hydraulic | 32 | ADC1_CH4 | ADC1 pin. |
| Battery sense | 33 | ADC1_CH5 | ADC1 pin. |
| Fuel level | 36 | ADC1_CH0 | Input-only; no internal pull-up. |
| Spare ADC | 39 | ADC1_CH3 | Input-only; no internal pull-up. |

## Digital I/O

| Function | GPIO | Notes |
|----------|------|-------|
| Status LED | 25 | Onboard LED (LedLink). **Do not reuse GPIO25 for a seat-bar switch.** |
| Buzzer | 26 | Spare header pin, digital output. |
| Seat bar interlock (optional) | 27 | Spare header pin, `INPUT_PULLUP` switch to ground. |
| Neutral safety interlock (optional) | 13 | Spare header pin, `INPUT_PULLUP` switch to ground. |

> **GPIO conflict fixed:** The old firmware used `SEAT_BAR = GPIO25`, which collides with the onboard status LED. Use a spare header pin such as **GPIO27** for the seat bar instead.

## Mandatory electrical protection

### Flyback diodes

- **1N4007 across the K3 starter solenoid coil** (cathode to coil +, anode to coil –/ground).
- **1N4007 across every external relay coil** driven by K2 (glow) and K4 (lights).
- Orient the cathode to the switched + side so the diode is reverse-biased in normal operation and clamps inductive kickback.

### Fusing

- **T-Relay Vcc:** 1–2 A fuse or PTC for the ESP32/relay board supply.
- **Per-circuit fusing:** size each external relay/load circuit independently:
  - Glow-plug power circuit: fuse to factory rating or wire gauge.
  - Starter solenoid: fuse to solenoid coil rating.
  - Lights: fuse per lamp load.

### Grounding

- **Single-point ground:** Bring all ground returns (battery, external relays, solenoid, sensor dividers, ESP32 GND) to one common chassis/star point. Avoid ground loops that can inject voltage drops into analog readings.

## Strapping / pin-use rules

- Avoid using pins 2, 4, 12, and 15 for user I/O. In particular, **GPIO12 is the MTDI flash-voltage strap** and must be held LOW at boot.
- If you need a spare input, prefer pins 13, 14, 22, 23, 26, 27, 32, 33.
- Analog sensors must be assigned to the ADC1 list: 32, 33, 34, 35, 36, 39.
- See [PINMAP.md](PINMAP.md) for the authoritative, settings-driven pin map and remapping rules.
