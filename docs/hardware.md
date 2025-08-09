# Hardware

**🚨 SAFETY CRITICAL - READ [SAFETY_CRITICAL.md](SAFETY_CRITICAL.md) FIRST 🚨**

**⚠️ CURRENT DESIGN HAS SAFETY VIOLATIONS - SEE SAFETY_CRITICAL.md**

Board: LILYGO T-Relay ESP32 (4 relays) - **HRS4H-S-DC5V relays rated 10A maximum DC**

## ❌ CRITICAL SAFETY ISSUES IN CURRENT DESIGN

- **GLOW PLUGS (GPIO19)**: 40-80A load exceeds 10A relay rating by 8x - **FIRE/EXPLOSION HAZARD**
- **LIGHTS (GPIO5)**: 9.2A load with insufficient safety margin - relay failure risk
- **MISSING SAFETY INTERLOCKS**: No seat bar or neutral safety switches

**MANDATORY**: Use T-Relay to pilot external relays for glow plugs and lights ONLY

# Hardware

**🚨 SAFETY CRITICAL - READ [SAFETY_CRITICAL.md](SAFETY_CRITICAL.md) FIRST 🚨**

**⚠️ CURRENT DESIGN HAS SAFETY VIOLATIONS - SEE SAFETY_CRITICAL.md**

Board: LILYGO T-Relay ESP32 (4 relays) - **HRS4H-S-DC5V relays rated 10A maximum DC**

## ❌ CRITICAL SAFETY ISSUES IN CURRENT DESIGN

- **GLOW PLUGS (GPIO19)**: 40-80A load exceeds 10A relay rating by 8x - **FIRE/EXPLOSION HAZARD**
- **LIGHTS (GPIO5)**: 9.2A load with insufficient safety margin - relay failure risk
- **MISSING SAFETY INTERLOCKS**: No seat bar or neutral safety switches

**MANDATORY**: Use T-Relay to pilot external relays for glow plugs and lights ONLY

## Relay Outputs (Active HIGH) - CORRECTED DESIGN

- MAIN_POWER (Relay 1) → GPIO21 (✅ Safe - controls electrical distribution)
- GLOW_PLUGS (Relay 2) → GPIO19 (⚠️ PILOT ONLY - must control external factory relay coil)
- STARTER (Relay 3) → GPIO18 (✅ Safe with flyback diode - 5-15A starter solenoid)
- LIGHTS (Relay 4) → GPIO5 (⚠️ PILOT ONLY - must control external 30A automotive relay)

**MANDATORY Protection**:
- 1N4007 flyback diode across starter solenoid coil (cathode to +, anode to ground)
- 1N4007 flyback diode across each external relay coil

## 🚨 MANDATORY SAFETY INTERLOCKS

### Safety Input Pins (INPUT_PULLUP configuration)
- Seat Bar Safety → GPIO25 (normally open, closes to ground when operator seated)
- Neutral Safety → GPIO27 (normally open, closes to ground when transmission in neutral)

**CRITICAL**: System MUST check both conditions before allowing starter engagement

## Analog Inputs (ESP32 ADC1, 12-bit 0–4095, 0–3.3V)

### Battery Voltage (GPIO36 / ADC1_CH0)

**Research-validated configuration**:
- Measured divider: R1 = 56kΩ (from battery to ADC), R2 = 10kΩ (ADC to GND)  
- Measured calibration: 13.06V battery → 2.0V ADC → 2482 ADC reading
- Calibration constant: 0.00526 V/ADC unit
- Protection: Voltage divider limits input to safe 3.3V range

### Engine Temp NTC (GPIO39 / ADC1_CH3)

**Research findings P/N 6658818**:
- 10kΩ NTC thermistor to GND, 10kΩ pull-up to 3.3V, midpoint to ADC39
- **⚠️ REQUIRES MANUAL CHARACTERIZATION**: No public resistance-vs-temperature curve
- Method: Measure resistance at 20°C, 40°C, 60°C, 80°C, 100°C in controlled water bath
- Current formula: Temp = 150°C - (ADC × 0.040) - **PLACEHOLDER until characterized**

### Fuel Level Sender (GPIO35 / ADC1_CH7)

**Research findings - unknown resistance range**:
- **⚠️ REQUIRES MANUAL MEASUREMENT**: Could be 240-33Ω, 73-10Ω, or 0-90Ω standard
- Current configuration: 100Ω pull-up to 3.3V (optimal for unknown range)
- **MANDATORY**: Measure actual sender resistance at full and empty tank
- Calibration via web interface once measured range is known

## Digital Inputs (INPUT_PULLUP configuration)

### Status Inputs
- Alternator Charge → GPIO22 (alternator "L" terminal via voltage divider)
  - Circuit: 10kΩ + 3.3kΩ divider with 3.6V Zener protection
  - 0V = not charging, ~14V = charging (scaled to 3.3V max)
- Engine Run Feedback → GPIO26 (internal pull-up, active-low)

### Pressure Switches (Digital, NOT Analog)
**Research findings - these are switches, not variable senders**:
- Oil Pressure Switch → GPIO34 (P/N 6969775, normally open, closes to ground when pressure OK)
- Hydraulic Pressure Switch → GPIO33 (P/N 6671062, normally open, closes to ground when pressure OK)

**Connection**: Direct to GPIO pins with INPUT_PULLUP (no external resistors needed)

### Power Management
- Wake Up Button → GPIO0 (BOOT button)
- Sleep Enable → GPIO12

## Signal Conditioning Summary

| Pin | Sensor Type | Circuit | Pull-up/Divider | Notes |
|-----|-------------|---------|-----------------|-------|
| GPIO39 | NTC Thermistor | Voltage divider | 10kΩ to 3.3V | Requires characterization |
| GPIO36 | Battery voltage | Voltage divider | 56kΩ + 10kΩ | Research-validated values |
| GPIO35 | Fuel sender | Voltage divider | 100Ω to 3.3V | Requires measurement |
| GPIO34 | Oil pressure switch | Direct | Internal pull-up | Digital switch, not analog |
| GPIO33 | Hydraulic pressure switch | Direct | Internal pull-up | Digital switch, not analog |
| GPIO25 | Seat bar switch | Direct | Internal pull-up | MANDATORY safety interlock |
| GPIO27 | Neutral switch | Direct | Internal pull-up | MANDATORY safety interlock |
| GPIO22 | Alternator signal | Voltage divider | 10kΩ + 3.3kΩ + Zener | 12V automotive to 3.3V logic |
| GPIO26 | Engine run feedback | Direct | Internal pull-up | Status feedback |

## Grounding & Protection

- **Single-point grounding**: All system grounds to single chassis point
- **Flyback diodes**: 1N4007 across all inductive loads (starter solenoid, external relay coils)
- **Fusing**: 2A fuse for ESP32 board power, appropriately sized fuses for all circuits
- **Enclosure**: Weather-resistant IP65 enclosure for T-Relay board

## Notes

- All relays default OFF at boot for safety
- Emergency stop immediately drops all relays
- Factory ignition switch MUST remain as backup/override
- See schematics_corrected.mmd for complete safety-compliant wiring diagram
