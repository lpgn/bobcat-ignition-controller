# Wiring Guide

Complete wiring instructions for the Bobcat Ignition Controller installation.

## Overview

This guide provides step-by-step wiring instructions for connecting the LILYGO T-Relay ESP32 board to a Bobcat 743 skid steer's electrical system. All connections use the 4-relay configuration with proper safety considerations.

## Safety Warnings

⚠️ **IMPORTANT SAFETY NOTES** ⚠️

- **Disconnect battery** before making any electrical connections
- **Verify polarity** on all power connections (reverse polarity can damage components)
- **Use proper fuses** on all power circuits
- **Test continuity** before applying power
- **Engine cannot be stopped electronically** - manual fuel shutoff only
- **Never bypass safety circuits** or engine protection systems

## Required Tools and Materials

### Tools

- Digital multimeter
- Wire strippers (16-12 AWG)
- Crimping tool for automotive connectors
- Heat gun (for heat shrink tubing)
- Drill with appropriate bits
- Label maker or permanent markers

### Materials

- 16 AWG automotive wire (red, black, white, blue, green, yellow, brown)
- Weather-resistant automotive connectors (Deutsch DT or equivalent)
- Heat shrink tubing (various sizes)
- Automotive fuses: 5A, 10A, 15A, 20A
- Fuse holders (inline automotive type)
- Ring terminals for battery connections
- Wire ties and protective conduit

## Power Distribution

### Main Power Feed

#### Battery Connection (12V Supply)

```text
Battery Positive (+) Terminal
    ↓
[20A Fuse] ← Master system fuse
    ↓
Terminal Block (Power Distribution)
    ↓
LILYGO T-Relay 12V Input (VIN)
```

**Wiring Details:**

- **Wire Gauge**: 12 AWG minimum for main power feed
- **Fuse Rating**: 20A fast-blow automotive fuse
- **Connection**: Ring terminal crimped to 12 AWG red wire
- **Protection**: Inline fuse holder near battery terminal

### Ground Connection

```text
Battery Negative (-) Terminal
    ↓
Chassis Ground Point
    ↓
Terminal Block (Ground Distribution)
    ↓
LILYGO T-Relay GND
```

**Wiring Details:**

- **Wire Gauge**: 12 AWG minimum for ground feed
- **Connection**: Ring terminal crimped to 12 AWG black wire
- **Grounding**: Use existing chassis ground point near battery

## Relay Output Wiring

### Relay 1: Main Power Control (GPIO5)

**Function**: Master power switch for all downstream electrical systems

```text
LILYGO Relay 1 Common ← +12V from battery (via 15A fuse)
LILYGO Relay 1 NO → Main power bus (to glow plugs, starter, lights)
LILYGO Relay 1 NC → Not connected
```

**Bobcat Connection Points:**
- **Common**: Direct connection to battery positive (through fuse)
- **NO Output**: Connect to main electrical bus or ignition switch input
- **Wire Gauge**: 14 AWG (rated for 15A continuous)
- **Fuse**: 15A automotive fuse in line

### Relay 2: Glow Plug Control (GPIO21)

**Function**: Activates diesel glow plugs for cold starting (20-second timer)

```text
LILYGO Relay 2 Common ← +12V from Main Power Relay output
LILYGO Relay 2 NO → Glow plug controller/solenoid
LILYGO Relay 2 NC → Not connected
```

**Bobcat Connection Points:**
- **Target Wire**: Glow plug controller input (usually orange/white wire)
- **Location**: Near engine firewall or fuse box
- **Current**: ~8A maximum (multiple glow plugs in parallel)
- **Wire Gauge**: 14 AWG
- **Fuse**: 10A automotive fuse

### Relay 3: Starter Control (GPIO22)

**Function**: Activates starter motor solenoid

```text
LILYGO Relay 3 Common ← +12V from Main Power Relay output
LILYGO Relay 3 NO → Starter solenoid control terminal
LILYGO Relay 3 NC → Not connected
```

**Bobcat Connection Points:**
- **Target Wire**: Starter solenoid "S" terminal (usually yellow/black wire)
- **Location**: Starter motor solenoid on engine
- **Current**: ~3A (solenoid coil only, not starter motor current)
- **Wire Gauge**: 16 AWG
- **Fuse**: 5A automotive fuse

### Relay 4: Combined Lights (GPIO18)

**Function**: Controls both front and rear work lights simultaneously

```text
LILYGO Relay 4 Common ← +12V from Main Power Relay output
LILYGO Relay 4 NO → Front and rear work light feeds (parallel)
LILYGO Relay 4 NC → Not connected
```

**Bobcat Connection Points:**
- **Front Lights**: Usually white/blue wire in front harness
- **Rear Lights**: Usually white/red wire in rear harness
- **Connection**: Join both light feeds to single relay output
- **Current**: ~6A total (assuming LED or efficient halogen lights)
- **Wire Gauge**: 14 AWG
- **Fuse**: 10A automotive fuse

## Sensor Input Wiring

### Engine Temperature Sensor (GPIO36)

**Signal Type**: 0-3.3V analog input via voltage divider

```text
Engine Temperature Sender
    ↓
[Voltage Divider Network]
    ↓
GPIO36 (ADC1_CH0)
```

**Wiring Details:**
- **Bobcat Connection**: Engine coolant temperature sender (usually green wire)
- **Signal Conditioning**: May require voltage divider for 0-5V sensors
- **Wire Gauge**: 22 AWG shielded (signal wire)
- **Connector**: Weather-resistant connector recommended

**Voltage Divider (if needed for 0-5V sensor):**
```text
+5V Sensor Output
    ↓
[10kΩ Resistor]
    ↓
GPIO36 ← (Tap point = 3.3V max)
    ↓
[6.8kΩ Resistor]
    ↓
Ground
```

### Oil Pressure Sensor (GPIO39)

**Signal Type**: 0-3.3V analog input

```text
Oil Pressure Sender
    ↓
[Signal Conditioning if needed]
    ↓
GPIO39 (ADC1_CH3)
```

**Wiring Details:**
- **Bobcat Connection**: Oil pressure sender (usually tan or brown wire)
- **Signal Range**: Typically 0-5V output, may need voltage divider
- **Wire Gauge**: 22 AWG shielded
- **Ground Reference**: Common ground with ESP32

### Battery Voltage Monitor (GPIO34)

**Signal Type**: 0-3.3V analog input via voltage divider

```text
+12V Battery Voltage
    ↓
[Voltage Divider 4:1 Ratio]
    ↓
GPIO34 (ADC1_CH6)
```

**Voltage Divider Network:**
```text
+12V Battery
    ↓
[30kΩ Resistor]
    ↓
GPIO34 ← (12V × 3/4 = 3V at 12V input)
    ↓
[10kΩ Resistor]
    ↓
Ground
```

**Wiring Details:**
- **Input Range**: 8-16V DC (covers battery voltage range)
- **Scaling**: 4:1 voltage divider (16V max input = 3.2V to GPIO)
- **Wire Gauge**: 22 AWG
- **Accuracy**: ±0.1V typical

### Fuel Level Sensor (GPIO35)

**Signal Type**: 0-3.3V analog input (if fuel level sender present)

```text
Fuel Level Sender (if equipped)
    ↓
[Signal Conditioning]
    ↓
GPIO35 (ADC1_CH7)
```

**Wiring Details:**
- **Bobcat Connection**: Fuel tank sender (varies by model)
- **Signal Type**: Usually variable resistance (240Ω-33Ω typical)
- **Conditioning**: May require pull-up resistor network
- **Wire Gauge**: 22 AWG

## Status Input Wiring

### Alternator Charge Indicator (GPIO27)

**Signal Type**: Digital input (active LOW)

```text
Alternator "L" Terminal (Lamp)
    ↓
[Pull-up Resistor if needed]
    ↓
GPIO27
```

**Wiring Details:**
- **Bobcat Connection**: Alternator lamp terminal (usually blue wire)
- **Logic**: LOW = charging, HIGH = not charging
- **Protection**: Internal pull-up resistor enabled in software
- **Wire Gauge**: 22 AWG

### Engine Run Feedback (GPIO14)

**Signal Type**: Digital input (active HIGH)

```text
Engine Oil Pressure Switch (or tachometer signal)
    ↓
[Signal Conditioning]
    ↓
GPIO14
```

**Wiring Details:**
- **Purpose**: Confirms engine is actually running
- **Signal Source**: Oil pressure switch or alternator W-terminal
- **Logic**: HIGH = engine running, LOW = engine stopped
- **Wire Gauge**: 22 AWG

## Connection Diagrams

### LILYGO T-Relay Terminal Assignments

```text
Power Input:
VIN  ← +12V (from battery via 20A fuse)
GND  ← Battery negative/chassis ground

Relay Outputs:
R1-C ← +12V battery (via 15A fuse)
R1-NO → Main power bus
R2-C ← +12V from R1-NO
R2-NO → Glow plug controller
R3-C ← +12V from R1-NO
R3-NO → Starter solenoid
R4-C ← +12V from R1-NO
R4-NO → Work lights (front + rear)

Sensor Inputs:
GPIO36 ← Engine temperature (0-3.3V)
GPIO39 ← Oil pressure (0-3.3V)
GPIO34 ← Battery voltage (0-3.3V via divider)
GPIO35 ← Fuel level (0-3.3V)

Status Inputs:
GPIO27 ← Alternator charge (digital)
GPIO14 ← Engine run feedback (digital)

Control Outputs:
GPIO5  → Relay 1 coil (main power)
GPIO21 → Relay 2 coil (glow plugs)
GPIO22 → Relay 3 coil (starter)
GPIO18 → Relay 4 coil (lights)
```

### Fuse and Protection Summary

| Circuit | Fuse Rating | Wire Gauge | Purpose |
|---------|-------------|------------|---------|
| Main Power Feed | 20A | 12 AWG | System power input |
| Main Power Relay | 15A | 14 AWG | Downstream power bus |
| Glow Plug Circuit | 10A | 14 AWG | Glow plug controller |
| Starter Circuit | 5A | 16 AWG | Starter solenoid |
| Light Circuit | 10A | 14 AWG | Work lights |

## Installation Steps

### Step 1: Planning and Preparation

1. **Study existing wiring** - Take photos of original connections
2. **Identify connection points** - Locate all target wires and terminals
3. **Plan routing** - Determine cable paths and mounting locations
4. **Gather materials** - Ensure all tools and supplies are available

### Step 2: Power Connections

1. **Disconnect battery** - Remove negative terminal first
2. **Install main fuse** - 20A fuse in line with positive feed
3. **Run power cable** - 12 AWG red wire from battery to controller location
4. **Run ground cable** - 12 AWG black wire to chassis ground point
5. **Connect LILYGO power** - VIN and GND terminals

### Step 3: Relay Output Connections

1. **Install relay fuses** - Individual fuses for each relay circuit
2. **Connect relay commons** - +12V feed to each relay common terminal
3. **Connect relay outputs** - Route to appropriate Bobcat systems
4. **Label all connections** - Mark each wire for future reference

### Step 4: Sensor Input Connections

1. **Install voltage dividers** - Build and test divider networks
2. **Connect sensor inputs** - Route sensor wires to GPIO pins
3. **Verify signal levels** - Use multimeter to confirm voltage ranges
4. **Test sensor operation** - Verify readings make sense

### Step 5: Testing and Verification

1. **Visual inspection** - Check all connections for proper termination
2. **Continuity testing** - Verify all circuits with multimeter
3. **Power-on test** - Apply power and check for proper operation
4. **Functional testing** - Test each relay and sensor input
5. **Load testing** - Verify proper operation under actual loads

## Troubleshooting

### Common Issues

#### No WiFi Connection

- Check ESP32 power supply (3.3V regulated)
- Verify antenna connection (if external)
- Check for interference from ignition system

#### Relays Not Switching

- Verify relay coil connections (GPIO pins)
- Check relay power supply (12V to commons)
- Test relay coils with multimeter

#### Incorrect Sensor Readings

- Verify voltage divider calculations
- Check sensor ground connections
- Calibrate sensor scaling in software

#### System Resets or Instability

- Check power supply quality (voltage ripple)
- Verify adequate current capacity (500mA minimum)
- Add filtering capacitors if needed

### Diagnostic Procedures

**Power Supply Test**
1. Measure voltage at LILYGO VIN terminal (should be 12V ±1V)
2. Measure voltage at 3.3V regulator output
3. Check for voltage drop under load

**Relay Function Test**
1. Use multimeter to verify relay coil resistance (typically 100-200Ω)
2. Apply 12V directly to relay coil to test mechanical operation
3. Verify contact continuity with relay energized

**Sensor Calibration**
1. Apply known voltages to sensor inputs
2. Compare readings in web interface
3. Adjust scaling factors in software if needed

## Maintenance

### Regular Inspections

- **Visual check** - Inspect for loose connections or corrosion (monthly)
- **Voltage verification** - Check power supply levels (quarterly)
- **Relay testing** - Verify proper switching operation (annually)
- **Sensor calibration** - Verify accuracy against known standards (annually)

### Environmental Protection

- **Moisture protection** - Ensure all connections are properly sealed
- **Vibration resistance** - Check mounting and connection security
- **Temperature monitoring** - Verify operation in extreme conditions
- **Corrosion prevention** - Apply dielectric grease to exposed connections

### Replacement Parts

Keep spare components on hand for critical systems:
- Automotive fuses (various ratings)
- Relay contacts (if serviceable)
- Connectors and terminals
- Wire and heat shrink tubing

## Documentation

### As-Built Documentation

After installation, document the actual wiring configuration:
- **Wire routing diagrams** - Show actual cable paths
- **Connection labels** - Record all wire numbers and destinations
- **Modification notes** - Document any changes from standard design
- **Test results** - Record initial calibration and test data

### Maintenance Records

Keep records of all maintenance activities:
- **Inspection dates and results**
- **Calibration adjustments**
- **Component replacements**
- **Problem reports and resolutions**
