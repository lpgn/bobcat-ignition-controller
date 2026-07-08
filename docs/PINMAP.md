# Bobcat 743 — Authoritative PINMAP

This is the settings-driven pin map for the **LilyGO T-Relay 4-Channel** ESP32-WROVER ignition controller. It documents the default GPIO for every function and the rules the config UI enforces.

## Pin table

| Function | Default GPIO | Type | Constraint / note |
|----------|--------------|------|-------------------|
| K1 — Main power relay | 21 | Relay (active-high) | **Fixed**. Onboard relay, 10 A max contact rating. |
| K2 — Glow plugs (pilot) | 19 | Relay (active-high) | **Fixed**. Pilots the factory high-current glow relay. Never drive glow plugs directly. |
| K3 — Starter (pilot) | 18 | Relay (active-high) | **Fixed**. Pilots the starter solenoid. 1N4007 flyback diode required across the coil. |
| K4 — Lights (pilot) | 5 | Relay (active-high) | **Fixed**. Pilots an external 30/40 A automotive relay. |
| Engine temperature | 34 | ADC | **ADC1 only** (32, 33, 34, 35, 36, 39). **Input-only**; no internal pull-up. |
| Oil pressure | 35 | ADC | **ADC1 only**; **input-only**. |
| Hydraulic | 32 | ADC | **ADC1 only**. |
| Battery sense | 33 | ADC | **ADC1 only**. |
| Fuel level | 36 | ADC | **ADC1 only**; **input-only**. |
| Spare ADC | 39 | ADC | **ADC1 only**; **input-only**. |
| Status LED / LedLink | 25 | Digital output | Onboard LED. **Reserved** — do not assign to a switch input. |
| Buzzer | 26 | Digital output | Spare header pin. |
| Seat bar interlock (optional) | 27 | Digital input | `INPUT_PULLUP`. User-remappable to any spare header pin. |
| Neutral safety interlock (optional) | 13 | Digital input | `INPUT_PULLUP`. User-remappable to any spare header pin. |

## Remapping rules

- **Relays are fixed.** K1–K4 are hard-wired to GPIO21, GPIO19, GPIO18, and GPIO5 on the T-Relay board. They cannot be remapped.
- **Sensors are user-remappable**, but they must be assigned to an **ADC1-only** pin while Wi-Fi is active: **{32, 33, 34, 35, 36, 39}**.
- **Digital I/O is user-remappable** to any spare, non-conflicting GPIO, with these exceptions:
  - **GPIO25 is reserved** for the onboard status LED and must not be selected for a switch input.
  - **Avoid strapping pins {2, 4, 12, 15}**. GPIO12 in particular must be LOW at boot (MTDI flash-voltage strap).
- **No duplicate GPIO assignments.** The config UI prevents two functions from using the same pin.

## Config UI enforcement

The settings page enforces this logic before accepting a new pin assignment:

1. **Sensor pins** must be in the ADC1 list: 32, 33, 34, 35, 36, 39.
2. **No duplicate GPIO** may be assigned to two functions.
3. **GPIO25 is blocked** for switch inputs (reserved for the onboard LED).
4. **Strapping-pin warning** if the user selects 2, 4, 12, or 15; recommend an alternative spare pin.

For wiring and protection details, see [hardware.md](hardware.md).
