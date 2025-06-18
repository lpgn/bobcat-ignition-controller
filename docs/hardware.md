# Bobcat Ignition Controller - Hardware Schematic

## System Overview

This document describes the hardware connections for the ESP32-based Bobcat ignition controller.

## Power Supply

- **Input**: 12V DC from Bobcat electrical system
- **ESP32 Power**: Use 12V to 3.3V regulator or USB power during development
- **Relay Power**: 12V DC for relay coils

## Relay Connections

### Glow Plug Relay (Relay 1)
```
ESP32 GPIO2 → Relay Driver → Relay Coil (+)
GND → Relay Coil (-)
12V+ → Relay Common (COM)
Glow Plug Circuit → Relay NO (Normally Open)
```

### Ignition/Starter Relay (Relay 2)
```
ESP32 GPIO4 → Relay Driver → Relay Coil (+)
GND → Relay Coil (-)
12V+ → Relay Common (COM)
Starter Circuit → Relay NO (Normally Open)
```

## Button Connections

### Start Button
```
ESP32 GPIO18 (with internal pullup enabled)
Button: One side to GPIO18, other side to GND
```

### Stop Button
```
ESP32 GPIO19 (with internal pullup enabled)
Button: One side to GPIO19, other side to GND
```

## Status Indicators

### Status LED
```
ESP32 GPIO5 → 220Ω Resistor → LED Anode
LED Cathode → GND
```

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
