# Bobcat Diesel Engine Controller - GPIO Connection Guide

## Overview
This document details the ESP32 GPIO connections for interfacing with a typical diesel engine's sensors and control systems. The design supports common diesel engine components found on Bobcat equipment.

## Power Supply Requirements
- **ESP32 Power**: 3.3V (regulated from 12V/24V system)
- **Relay Coils**: 12V/24V (depending on system voltage)
- **Sensor Power**: 5V regulated (for pressure/temperature sensors)

---

## Digital Output Pins (Relay Control)

### Primary Engine Control
| GPIO | Function | Relay Type | Load Current | Notes |
|------|----------|------------|--------------|-------|
| 21 | Glow Plugs | SPST 40A | 20-40A | Controls glow plug heating circuit |
| 23 | Ignition/Run | SPST 30A | 5-15A | Main ignition switch position |
| 22 | Starter | SPST 100A | 80-150A | Starter solenoid activation |

### Auxiliary Systems
| GPIO | Function | Relay Type | Load Current | Notes |
|------|----------|------------|--------------|-------|
| 19 | Fuel Pump | SPST 20A | 3-8A | Electric fuel pump (if equipped) |
| 18 | Cooling Fan | SPST 30A | 10-20A | Radiator cooling fan override |

---

## Analog Input Pins (Engine Sensors)

### Primary Engine Parameters
| GPIO | ADC Ch | Sensor Type | Range | Signal Type | Connection |
|------|--------|-------------|-------|-------------|------------|
| 36 | ADC1_CH0 | Coolant Temp | -40°C to 150°C | 0-5V | NTC Thermistor via signal conditioner |
| 39 | ADC1_CH3 | Oil Pressure | 0-689 kPa | 0-5V | Pressure transducer |
| 34 | ADC1_CH6 | Battery Voltage | 0-30V | 0-3.3V | Voltage divider (10:1 ratio) |
| 35 | ADC1_CH7 | Fuel Level | 0-100% | 0-5V | Resistive fuel sender |
| 32 | ADC1_CH4 | RPM (Analog) | 0-4000 RPM | 0-5V | Frequency-to-voltage converter |

### Sensor Wiring Details

#### Coolant Temperature Sensor
```
Engine Temp Sensor (2-wire)
├── Wire 1 (Signal) → Signal Conditioner → GPIO36
├── Wire 2 (Ground) → Engine Ground
└── Pull-up resistor: 4.7kΩ to +5V
```

#### Oil Pressure Sensor  
```
Oil Pressure Transducer (3-wire)
├── Red (+5V) → Regulated 5V supply
├── Black (GND) → Common ground
└── Blue/White (Signal) → GPIO39 (0-5V output)
```

#### Battery Voltage Monitor
```
Battery Voltage Divider
12V/24V Battery → [47kΩ] → GPIO34 → [4.7kΩ] → GND
                            ↑
                         3.3V max input
Ratio: (4.7kΩ)/(47kΩ + 4.7kΩ) = 0.091
```

---

## Digital Input Pins (Safety Switches)

### Safety Interlocks
| GPIO | Function | Switch Type | Logic | Connection |
|------|----------|-------------|-------|------------|
| 25 | Oil Pressure Switch | Normally Closed | Active LOW | Opens when pressure drops |
| 26 | Coolant Temp Switch | Normally Open | Active HIGH | Closes when overheated |
| 27 | Alternator Charge | Charge Light | Active LOW | Grounded when charging |
| 14 | Engine Run Feedback | Oil Pressure/RPM | Active HIGH | High when engine running |
| 12 | Emergency Stop | Push Button | Active LOW | Normally closed circuit |

### Switch Wiring Details

#### Oil Pressure Safety Switch
```
Low Oil Pressure Switch (Normally Closed)
Engine Block → Switch → GPIO25 + Pull-up (10kΩ to 3.3V)
When oil pressure OK: Switch closed → GPIO25 = LOW
When oil pressure low: Switch opens → GPIO25 = HIGH (ALARM)
```

#### Emergency Stop Circuit
```
Emergency Stop Button (Normally Closed)
+3.3V → [10kΩ pull-up] → GPIO12 → E-Stop Switch → GND
Normal operation: Switch closed → GPIO12 = LOW
Emergency: Switch opened → GPIO12 = HIGH (STOP)
```

---

## Pulse Input Pins (RPM Measurement)

### RPM Sensor Interface
| GPIO | Function | Sensor Type | Signal | Frequency Range |
|------|----------|-------------|--------|-----------------|
| 13 | RPM Sensor | Magnetic Pickup | AC Pulse | 0-200 Hz (0-4000 RPM) |
| 15 | Hour Meter | Pulse Counter | Digital | 1 pulse/revolution |

#### Magnetic Pickup RPM Sensor
```
Magnetic Pickup Coil (2-wire)
├── Signal+ → Signal conditioner → GPIO13
├── Signal- → Signal conditioner ground
└── Signal conditioner: Converts AC pulses to 3.3V logic levels
```

---

## Signal Conditioning Requirements

### Temperature Sensor Conditioning
```
NTC Thermistor → Wheatstone Bridge → Op-amp → 0-5V → Voltage Divider → GPIO
Required components:
- Reference resistor (matching thermistor at 25°C)
- Op-amp buffer (LM358 or similar)
- 5V to 3.3V voltage divider
```

### RPM Signal Conditioning
```
Magnetic Pickup → AC Coupling → Comparator → Digital Pulses → GPIO
Required components:
- Coupling capacitor (1µF)
- Bias resistors (47kΩ)
- Comparator (LM393)
- Pull-up resistor (10kΩ to 3.3V)
```

---

## Safety Considerations

### Critical Safety Interlocks
1. **Low Oil Pressure**: Immediate shutdown if oil pressure drops
2. **High Coolant Temperature**: Shutdown with cooldown sequence
3. **Emergency Stop**: Immediate all-systems shutdown
4. **Overspeed Protection**: RPM limit enforcement
5. **Battery Voltage**: Monitor for under/over voltage conditions

### Fail-Safe Design
- All safety switches use fail-safe logic (fail to safe state)
- Relays are normally open (engine stops if control fails)
- Emergency stop breaks all power circuits
- Watchdog timer resets system if software hangs

---

## Installation Notes

### Relay Installation
- Use automotive-grade relays rated for engine bay temperatures
- Install relays in weatherproof enclosure
- Use proper gauge wire for high-current circuits (starter relay)
- Include fuses in all power circuits

### Sensor Installation
- Route sensor wires away from ignition wires (EMI prevention)
- Use shielded cable for analog sensor signals
- Secure all connections with heat-shrink tubing
- Ground all sensor shields to chassis ground at one point only

### Environmental Protection
- IP65 rated enclosure minimum for ESP32 module
- Conformal coating on PCB for moisture protection
- Vibration-resistant mounting for all components
- Temperature range: -40°C to +85°C operation

---

## Troubleshooting

### Common Issues
1. **False RPM readings**: Check magnetic pickup gap (0.020" typical)
2. **Erratic temperature readings**: Verify thermistor connections
3. **Oil pressure alarms**: Check switch and wiring continuity
4. **Relay chatter**: Verify adequate power supply capacity
5. **EMI interference**: Add ferrite cores to sensor cables

### Diagnostic Features
- Web interface displays all sensor values in real-time
- Error codes for each fault condition
- Data logging for troubleshooting intermittent issues
- Built-in sensor calibration routines
