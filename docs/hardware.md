# Hardware Summary

Quick reference for the Bobcat Ignition Controller hardware configuration.

## Current Hardware Platform

**Controller Board**: LILYGO T-Relay ESP32 (4-channel relay version)

### Key Specifications
- **Microcontroller**: ESP32-WROOM-32
- **Relays**: 4x SPDT, 10A capacity each
- **Power Input**: 12V DC (7-30V range)
- **Communication**: WiFi 802.11 b/g/n (2.4GHz)
- **Programming**: USB-C connector

## 4-Relay Configuration

| Relay | GPIO | Function | Load Rating |
|-------|------|----------|-------------|
| 1 | GPIO5 | Main Power | 10A @ 12V |
| 2 | GPIO21 | Glow Plugs | 10A @ 12V |
| 3 | GPIO22 | Starter | 10A @ 12V |
| 4 | GPIO18 | Lights | 10A @ 12V |

## Sensor Inputs

| GPIO | Function | Input Type |
|------|----------|------------|
| GPIO36 | Engine Temperature | 0-3.3V ADC |
| GPIO39 | Oil Pressure | 0-3.3V ADC |
| GPIO34 | Battery Voltage | 0-3.3V ADC |
| GPIO35 | Fuel Level | 0-3.3V ADC |

## Status Inputs

| GPIO | Function | Signal Type |
|------|----------|-------------|
| GPIO27 | Alternator Status | Digital (Active LOW) |
| GPIO14 | Engine Run | Digital (Active HIGH) |

## System Constraints

### Design Limitations
- **No Engine Stop**: Cannot electronically stop engine (manual lever only)
- **4-Relay Maximum**: Limited by LILYGO T-Relay board capacity
- **Safety Alerts Only**: Visual warnings, no automatic shutdowns
- **12V System**: Designed for 12V Bobcat electrical systems

### Key Features
- **Combined Lighting**: Single relay controls front and back work lights
- **Glow Plug Timer**: 20-second automated preheating cycle
- **Web Interface**: Mobile-responsive control via WiFi (192.168.4.1)
- **Real-time Monitoring**: Engine temperature, oil pressure, battery voltage

## Complete Documentation

For detailed information, refer to these comprehensive documents:

### Hardware Reference
- **[Hardware Overview](hardware_overview.md)** - Complete specifications and requirements
- **[Board Pinout](board_pinout.md)** - Detailed GPIO assignments and electrical specs
- **[Wiring Guide](wiring_guide.md)** - Step-by-step installation instructions

### System Documentation
- **[GPIO Connections](gpio_connections.md)** - Pin assignments and interface details
- **[Schematics](schematics.mmd)** - System diagrams and block diagrams
- **[System Control Diagram](system_control_diagram.md)** - Logic and state machine

## Target Equipment

**Primary Target**: Bobcat 743 Skid Steer
- **Engine**: Kubota V1702-BA (4-cylinder diesel)
- **Electrical**: 12V negative ground system
- **Glow Plugs**: 12V ceramic fast-heat type
- **Engine Stop**: Manual fuel shutoff lever (not electronic)

## Installation Requirements

### Minimum Requirements
- LILYGO T-Relay ESP32 board
- 12V power connection to Bobcat electrical system
- Weather-resistant mounting enclosure
- Automotive-grade wiring and connectors
- Individual fuses for each relay circuit

### Optional Components
- Engine temperature sensor (if not equipped)
- Oil pressure sender (if not equipped)
- Fuel level sender (if not equipped)
- External status indicators
- Emergency stop switch (power cut only)

## Safety Considerations

⚠️ **Important Safety Notes**

1. **Engine Stop**: Engine must be stopped manually - cannot be stopped electronically
2. **Fuse Protection**: All relay outputs must be individually fused
3. **Power Disconnect**: Always disconnect battery before making connections
4. **Alert System**: Safety alerts are visual only - no automatic shutdowns
5. **Manual Override**: Always maintain manual control capability

## Quick Start

1. **Read Documentation**: Start with [Hardware Overview](hardware_overview.md)
2. **Plan Installation**: Review [Wiring Guide](wiring_guide.md)
3. **Prepare Hardware**: Gather components and tools
4. **Install Safely**: Follow safety procedures and fusing requirements
5. **Test System**: Verify all functions before operational use

For complete installation and operation instructions, see the comprehensive documentation referenced above.

### Buzzer (Optional)
```
ESP32 GPIO21 → Buzzer (+)
Buzzer (-) → GND
```

## Sensor Connections

### Battery Voltage Monitor
```
12V Battery (+) → R1 (10kΩ) → ESP32 GPIO32 → R2 (3.3kΩ) → GND
Voltage divider: Vout = Vin × (R2 / (R1 + R2))
Max input: 12V → 3.0V output (safe for ESP32)
```

### Engine Temperature Sensor
```
3.3V → R1 (10kΩ) → ESP32 GPIO34
GPIO34 → NTC Thermistor → GND
(Standard NTC temperature sensor circuit)
```

### Oil Pressure Sensor
```
Pressure Sensor 0-5V Output → Voltage Divider → ESP32 GPIO35
(If sensor outputs 5V, use voltage divider to scale to 3.3V max)
5V → R1 (2.2kΩ) → ESP32 GPIO35 → R2 (3.3kΩ) → GND
```

## Protection Circuits

### Fuse Protection
- Main power: 10A fuse
- Glow plug circuit: 30A fuse (check Bobcat specifications)
- Starter circuit: 30A fuse (check Bobcat specifications)

### Flyback Diodes
Add flyback diodes across relay coils:
```
Relay Coil (+) ← Cathode | Diode | Anode → Relay Coil (-)
```

### ESD Protection
- Add ESD protection diodes on all input pins
- Use optoisolated relay drivers for high-current switching

## Connector Pinout

### Main Harness Connector (Suggested)
1. 12V Power (+)
2. Ground (-)
3. Glow Plug Control Output
4. Starter Control Output
5. Engine Temperature Input
6. Oil Pressure Input
7. Start Button Input
8. Stop Button Input

## PCB Layout Considerations

1. **Isolation**: Keep high-current relay circuits isolated from ESP32
2. **Ground Plane**: Use solid ground plane for noise reduction
3. **Decoupling**: Add decoupling capacitors (100nF) near ESP32 power pins
4. **Protection**: Include TVS diodes for automotive environment protection
5. **Connector**: Use automotive-grade connectors with proper sealing

## Testing Points

Add test points for debugging:
- TP1: 3.3V Supply
- TP2: 12V Supply  
- TP3: Glow Plug Control Signal
- TP4: Ignition Control Signal
- TP5: Battery Voltage Sense
- TP6: Temperature Sensor
- TP7: Oil Pressure Sensor

## Assembly Notes

1. Use automotive-grade components rated for temperature extremes
2. Conformal coat PCB for moisture protection
3. Mount in weatherproof enclosure
4. Secure all connections with proper crimps and heat shrink
5. Label all wires for easy maintenance

## Safety Considerations

⚠️ **WARNING**: This system controls high-current automotive circuits
- Always disconnect battery before working on electrical systems
- Use proper fuses and circuit protection
- Test all safety systems before first use
- Follow all automotive electrical safety practices
