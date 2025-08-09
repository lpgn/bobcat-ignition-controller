# GPIO Connection Guide

Complete reference for ESP32 GPIO pin assignments in the Bobcat Ignition Controller using the LILYGO T-Relay 4-channel board.

## Overview

This document provides the definitive GPIO pin mapping for the current 4-relay controller design. All pin assignments have been optimized for the LILYGO T-Relay hardware and the specific requirements of Bobcat equipment with manual engine stop.

## Key Design Principles

- **4-Relay Configuration**: Optimized for LILYGO T-Relay board limitations
- **Manual Engine Stop**: No electronic engine shutdown capability
- **Safety-First**: Visual alerts only, no automatic shutdowns
- **Combined Lighting**: Single relay controls all work lights

## Power Supply Connections

| Terminal | Voltage | Function | Current Capacity |
|----------|---------|----------|------------------|
| VIN | 12V DC | Main power input | 2A maximum |
| GND | 0V | System ground | - |
| 3V3 | 3.3V | ESP32 supply (regulated) | 600mA |
| 5V | 5V | Sensor excitation (regulated) | 500mA |

## Digital Output Pins (Relay Control)

### Active Relay Assignments - LILYGO T-Relay 4-Channel ESP32

| GPIO | Pin Name | Relay | Function | Load Rating | Purpose |
|------|----------|-------|----------|-------------|---------|
| GPIO21 | D21 | Relay 1 | Main Power | 10A @ 12V | Master electrical system control |
| GPIO19 | D19 | Relay 2 | Glow Plugs | 10A @ 12V | Diesel preheating (20-sec timer) |
| GPIO18 | D18 | Relay 3 | Starter | 10A @ 12V | Starter solenoid activation |
| GPIO5 | D5 | Relay 4 | Lights | 10A @ 12V | Combined work lights |

### Relay Control Logic

- **Active HIGH**: GPIO output HIGH energizes relay coil
- **Switching Delay**: ~10ms mechanical switching time
- **Coil Resistance**: ~150Ω per relay coil
- **LED Indicators**: Green LED on board shows relay status

## Analog Input Pins (Sensor Monitoring)

### Engine Sensor Inputs - Physical sequence from development header

| GPIO | ADC Channel | Function | Input Range | Physical Position |
|------|-------------|----------|-------------|-------------------|
| GPIO39 | ADC1_CH3 | Engine Temperature | 0-3.3V | Top row, 4th pin |
| GPIO35 | ADC1_CH7 | Oil Pressure | 0-3.3V | Top row, 5th pin |
| GPIO36 | ADC1_CH0 | Battery Voltage | 0-3.3V | Bottom row, 4th pin |
| GPIO34 | ADC1_CH6 | Fuel Level | 0-3.3V | Bottom row, 5th pin |

### Signal Conditioning Requirements

#### Engine Temperature (GPIO36)
```text
Engine Coolant Temp Sender
    ↓
[Voltage Divider/Signal Conditioner]
    ↓
GPIO36 (0-3.3V range)
```

- **Sensor Type**: NTC thermistor or linear voltage output
- **Input Protection**: 3.3V clamping, 100nF filter capacitor
- **Calibration**: Software scaling for specific sensor characteristics

#### Oil Pressure (GPIO39)
```text
Oil Pressure Sender
    ↓
[Signal Conditioner if needed]
    ↓
GPIO39 (0-3.3V range)
```

- **Sensor Type**: Resistive sender or voltage transducer
- **Range**: 0-100 PSI (0-689 kPa)
- **Alert Threshold**: <10 PSI triggers warning

#### Battery Voltage (GPIO34)
```text
+12V Battery
    ↓
[Voltage Divider 4:1]
    ↓
GPIO34 (max 3.2V at 16V input)
```

- **Voltage Divider**: 30kΩ / 10kΩ resistors
- **Input Range**: 8-16V DC (covers normal battery range)
- **Alert Levels**: <11V (low), >15V (overcharge)

#### Fuel Level (GPIO35)
```text
Fuel Tank Sender
    ↓
[Pull-up resistor network]
    ↓
GPIO35 (0-3.3V range)
```

- **Sender Type**: Variable resistance (240Ω-33Ω typical)
- **Signal Conditioning**: May require pull-up resistor
- **Range**: 0-100% fuel level

## Digital Input Pins (Status Monitoring)

### Engine Status Inputs - Physical sequence from development header

| GPIO | Function | Signal Type | Logic Level | Physical Position |
|------|----------|-------------|-------------|-------------------|
| GPIO22 | Alternator Charge | Digital | Active LOW | Top row, 1st pin |
| GPIO26 | Engine Run Feedback | Digital | Active HIGH | Top row, 2nd pin |

### Input Specifications

#### Alternator Charge Status (GPIO22)

```text
Alternator "L" Terminal
    ↓
[Optional signal conditioning]
    ↓
GPIO22 (with internal pull-up)
```

- **Signal Source**: Alternator lamp terminal (typically blue wire)
- **Logic**: LOW = charging, HIGH = not charging
- **Protection**: Internal pull-up resistor enabled
- **Physical Position**: Top row, 1st pin

#### Engine Run Feedback (GPIO26)

```text
Oil Pressure Switch or Tach Signal
    ↓
[Signal conditioning if needed]
    ↓
GPIO26
```

- **Physical Position**: Top row, 2nd pin

- **Purpose**: Confirms engine is actually running
- **Signal Options**: Oil pressure switch, alternator W-terminal, or tach signal
- **Logic**: HIGH = engine running, LOW = engine stopped
- **Timeout**: Used for safety interlocks

## Unused/Available Pins

### Available for Future Expansion (From LILYGO T-Relay Header)

| GPIO | Capabilities | Restrictions | Suggested Use |
|------|--------------|--------------|---------------|
| GPIO2 | Digital I/O, LED | Used for WiFi status LED | Additional status output |
| GPIO4 | Digital I/O | - | External alarm output |
| GPIO12 | Digital I/O | Must be LOW at boot | Expansion input |
| GPIO13 | Digital I/O | - | Additional sensor input |
| GPIO15 | Digital I/O | Must be LOW at boot | Expansion output |
| GPIO22 | Digital I/O | - | Serial communication (TX) |
| GPIO23 | Digital I/O | - | Serial communication (RX) |
| GPIO26 | Digital I/O, DAC | - | Analog output if needed |
| GPIO32 | Digital I/O, ADC | - | Additional analog input |
| GPIO33 | Digital I/O, ADC | - | Additional analog input |

**Available ADC pins**: GPIO32, GPIO33, GPIO34, GPIO35, GPIO36, GPIO39
**Note**: GPIO27 and GPIO14 are used for status inputs

### Expansion Considerations

- **Current Limitation**: 12mA maximum per GPIO pin
- **Voltage Level**: 3.3V logic levels only (not 5V tolerant)
- **Total Current**: 200mA maximum for all GPIO pins combined
- **Boot Strapping**: Some pins have boot-time restrictions

## Communication Interfaces

### WiFi (Built-in)
- **Standard**: 802.11 b/g/n (2.4GHz only)
- **Range**: 50-100 meters line of sight
- **Security**: WPA2/WPA3 encryption supported
- **Access Point**: Can create hotspot for direct connection

### USB Programming Port
- **Connector**: USB-C
- **Chip**: CH340 USB-to-serial converter
- **Baud Rate**: 115200 bps (default)
- **Auto-programming**: Automatic boot mode selection

### Future Communication Options

#### CAN Bus Interface (if added)

- **Potential Pins**: GPIO22 (TX), GPIO23 (RX)
- **Transceiver**: External CAN transceiver IC required
- **Protocol**: CANopen or J1939 for heavy equipment

#### RS485 Interface (if added)

- **Potential Pins**: GPIO32 (TX), GPIO33 (RX)
- **Transceiver**: External RS485 transceiver required
- **Use**: Modbus communication with external systems

## Wiring Harness Connections

### Main Power Harness

| Wire Color | Function | Gauge | Fuse | Destination |
|------------|----------|-------|------|-------------|
| Red | +12V Main | 12 AWG | 20A | Battery positive |
| Black | Ground | 12 AWG | - | Chassis ground |
| Yellow | Relay Commons | 14 AWG | 15A | Power distribution |

### Relay Output Harness

| Wire Color | Function | GPIO | Gauge | Fuse | Bobcat Connection |
|------------|----------|------|-------|------|-------------------|
| Orange | Main Power | GPIO21 | 14 AWG | 15A | Main electrical bus |
| Blue | Glow Plugs | GPIO19 | 14 AWG | 10A | Glow plug controller |
| Purple | Starter | GPIO18 | 16 AWG | 5A | Starter solenoid |
| White | Lights | GPIO5 | 14 AWG | 10A | Work light circuits |

### Sensor Input Harness

| Wire Color | Function | GPIO | Gauge | Shielding | Bobcat Connection |
|------------|----------|------|-------|-----------|-------------------|
| Green | Engine Temp | GPIO39 | 22 AWG | Yes | Coolant temp sender |
| Brown | Oil Pressure | GPIO35 | 22 AWG | Yes | Oil pressure sender |
| Gray | Battery Voltage | GPIO36 | 22 AWG | No | Battery positive tap |
| Pink | Fuel Level | GPIO34 | 22 AWG | Yes | Fuel tank sender |

### Status Input Harness

| Wire Color | Function | GPIO | Gauge | Bobcat Connection |
|------------|----------|------|-------|-------------------|
| Light Blue | Alternator | GPIO22 | 22 AWG | Alternator L terminal |
| Violet | Engine Run | GPIO26 | 22 AWG | Oil pressure switch |

## Installation Guidelines

### Routing Recommendations

1. **Power Wires**: Route separately from signal wires
2. **Sensor Wires**: Use shielded cable for analog inputs
3. **Ground Loops**: Single-point grounding system
4. **Protection**: Fuse all power circuits appropriately

### Connection Best Practices

1. **Crimped Connections**: Use proper automotive crimp terminals
2. **Weather Sealing**: Use weatherproof connectors outdoors
3. **Strain Relief**: Provide adequate strain relief at connections
4. **Labeling**: Label all wires for future maintenance

### Testing Procedures

1. **Continuity Check**: Verify all connections before power-on
2. **Voltage Verification**: Check all power supply levels
3. **Sensor Testing**: Verify sensor readings make sense
4. **Relay Testing**: Confirm all relays switch properly

## Troubleshooting Reference

### Common GPIO Issues

**Relay Not Switching**
- Check GPIO output voltage (should be 3.3V when active)
- Verify relay coil continuity (~150Ω)
- Confirm adequate power supply to relay coils

**Sensor Reading Problems**
- Check input voltage levels (should be 0-3.3V)
- Verify sensor power supply and ground connections
- Look for electrical noise on sensor lines

**Communication Issues**
- Verify WiFi antenna placement
- Check for interference from ignition system
- Confirm adequate power supply to ESP32

### Diagnostic Tools

- **Multimeter**: Essential for voltage and continuity testing
- **Oscilloscope**: Helpful for analyzing signal quality
- **WiFi Analyzer**: Check for interference and signal strength
- **Serial Monitor**: Debug software issues via USB connection

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
