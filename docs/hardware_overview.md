# Hardware Overview

Complete hardware specifications and requirements for the Bobcat Ignition Controller.

## System Architecture

The Bobcat Ignition Controller is built around the LILYGO T-Relay ESP32 board, providing web-based control for older Bobcat equipment where the engine cannot be stopped electronically.

### Key Design Principles

- **Safety First**: Visual alerts only - cannot auto-shutdown engine
- **Manual Engine Stop**: Engine must be stopped with manual lever
- **4-Relay Design**: Optimized for LILYGO T-Relay hardware limitations
- **Web Interface**: Modern responsive control panel accessible from any device

## Hardware Requirements

### Primary Controller Board

#### LILYGO T-Relay ESP32 Development Board

- **Part Number**: LILYGO T-Relay (4-channel relay version)
- **Microcontroller**: ESP32-WROOM-32
- **Operating Voltage**: 3.3V (powered from 12V input)
- **Relay Configuration**: 4x SPDT relays, 10A capacity each
- **Power Input**: 7-30V DC (12V recommended)
- **WiFi**: 802.11 b/g/n (2.4GHz)
- **Programming**: USB-C connector with CH340 USB-to-Serial

### Relay Assignments

| Relay | GPIO | Function | Load | Description |
|-------|------|----------|------|-------------|
| 1 | GPIO5 | Main Power | 5A | Master power control for all electrical systems |
| 2 | GPIO21 | Glow Plugs | 8A | Diesel glow plug preheating (20-second cycle) |
| 3 | GPIO22 | Starter Motor | 3A | Starter solenoid relay control signal |
| 4 | GPIO18 | Lights | 6A | Combined front and back work lights |

### Sensor Inputs

| GPIO | ADC Channel | Sensor | Range | Signal Type |
|------|-------------|--------|-------|-------------|
| GPIO36 | ADC1_CH0 | Engine Temperature | -40°C to 150°C | 0-3.3V via voltage divider |
| GPIO39 | ADC1_CH3 | Oil Pressure | 0-689 kPa | 0-3.3V via voltage divider |
| GPIO34 | ADC1_CH6 | Battery Voltage | 8-16V | 0-3.3V via voltage divider (4:1) |
| GPIO35 | ADC1_CH7 | Fuel Level | 0-100% | 0-3.3V via voltage divider |

### Status Inputs

| GPIO | Function | Signal Type | Description |
|------|----------|-------------|-------------|
| GPIO27 | Alternator Charge | Digital (Active LOW) | Charging system status |
| GPIO14 | Engine Run Feedback | Digital (Active HIGH) | Engine running confirmation |

## Power Supply Design

### Input Power

- **Source**: Bobcat 12V electrical system
- **Input Range**: 10.5V - 16V DC (accommodates voltage variations)
- **Protection**: Recommended fuse and reverse polarity protection
- **Current Draw**: ~500mA @ 12V (ESP32 + relays)

### Voltage Regulation

- **ESP32 Power**: Onboard 3.3V regulator (handles 12V input)
- **Relay Power**: Direct 12V feed to relay coils
- **Sensor Power**: 5V regulated output for sensor excitation (if needed)

## Environmental Specifications

### Operating Conditions

- **Temperature Range**: -40°C to +85°C (industrial grade)
- **Humidity**: 0-95% non-condensing
- **Vibration**: Automotive environment compatible
- **IP Rating**: IP54 minimum (with proper enclosure)

### Installation Requirements

- **Mounting**: DIN rail or panel mount in engine compartment
- **Enclosure**: Weather-resistant, ventilated for heat dissipation
- **Wiring**: Automotive-grade wire (minimum 16 AWG for power)
- **Connectors**: Weather-resistant automotive connectors recommended

## Bobcat Engine Interface

### Target Equipment

**Primary Target**: Bobcat 743 Skid Steer

- **Engine**: Kubota V1702-BA (4-cylinder diesel)
- **Electrical System**: 12V negative ground
- **Glow Plug System**: 12V fast-heat ceramic plugs
- **Starter**: Heavy-duty 12V starter motor
- **Engine Stop**: Manual fuel shutoff lever (not electronically controlled)

### Engine Sensors

#### Temperature Sensor

- **Type**: NTC thermistor or linear voltage output
- **Location**: Engine coolant passage
- **Signal**: 0-5V or resistance-based
- **Calibration**: Field calibration required for specific sensor

#### Oil Pressure Sensor

- **Type**: Resistive sender or voltage output transducer
- **Range**: 0-100 PSI (0-689 kPa)
- **Signal**: Variable resistance or 0-5V
- **Warning Point**: <10 PSI (69 kPa)

#### Battery Monitoring

- **Voltage Range**: 8V-16V (12V nominal)
- **Monitoring**: Charging system status and battery condition
- **Alert Levels**: <11V (low) / >15V (overcharge)

## Relay Wiring

### High-Current Connections

All relay contacts handle the actual load currents to the Bobcat systems:

#### Main Power Relay (GPIO5)

```text
Common: +12V from battery
NO: To all downstream systems (lights, ignition)
NC: Not used
```

#### Glow Plug Relay (GPIO21)

```text
Common: +12V from main power relay
NO: To glow plug controller input
NC: Not used
```

#### Starter Relay (GPIO22)

```text
Common: +12V from main power relay
NO: To starter solenoid coil
NC: Not used
```

#### Lights Relay (GPIO18)

```text
Common: +12V from main power relay
NO: To front and back work lights
NC: Not used
```

### Safety Considerations

- **Fuse Protection**: Each relay output should be individually fused
- **Wire Gauge**: Minimum 14 AWG for relay contacts, 16 AWG for control signals
- **Relay Ratings**: 10A relays adequate for control signals (not direct motor loads)
- **Heat Dissipation**: Ensure adequate ventilation in enclosed installation

## Optional Accessories

### External Sensors

- **Fuel Level Sensor**: Resistive float sender (if not equipped)
- **RPM Sensor**: Magnetic pickup or alternator W-terminal
- **Hour Meter**: External pulse counter integration

### User Interface

- **Emergency Stop**: External e-stop button (power cut only)
- **Status Lights**: External LED indicators for engine status
- **Audible Alarm**: Horn or buzzer for critical alerts

### Communication

- **CAN Bus**: Future expansion for diagnostic protocols
- **RS485**: Integration with external monitoring systems
- **Bluetooth**: Close-range mobile device connectivity

## Bill of Materials (Core Components)

| Component | Part Number | Quantity | Approximate Cost |
|-----------|-------------|----------|-----------------|
| LILYGO T-Relay Board | LILYGO T-Relay | 1 | $25-35 |
| Automotive Fuses | ATO/ATC 5A-15A | 6 | $5 |
| Terminal Blocks | Phoenix Contact | 1 set | $15 |
| Wire (16 AWG) | Various colors | 50 ft | $20 |
| Weatherproof Connectors | Deutsch DT series | 4 | $30 |
| Enclosure | IP54 Plastic | 1 | $25 |
| **Total Estimated Cost** | | | **$120-130** |

## Installation Tools Required

- **Multimeter**: For voltage verification and troubleshooting
- **Wire Strippers**: For professional wire termination
- **Crimping Tool**: For automotive connectors
- **Drill/Bits**: For mounting holes and cable entries
- **Heat Gun**: For heat shrink tubing
- **Label Maker**: For wire identification

## Performance Specifications

### Response Times

- **Power On**: <1 second
- **Glow Plug Activation**: Immediate
- **Web Interface**: <2 seconds to load
- **Sensor Reading Update**: 2-second intervals
- **Alert Response**: <1 second

### Reliability Metrics

- **Operating Life**: 5+ years in automotive environment
- **Relay Cycles**: 100,000+ switching cycles per relay
- **WiFi Range**: 50+ meters in open air
- **MTBF**: >50,000 hours continuous operation
