# Board Pinout Reference

Complete GPIO pin assignments and electrical specifications for the LILYGO T-Relay ESP32 board in the Bobcat Ignition Controller.

## Overview

This document provides the definitive reference for all pin assignments, electrical specifications, and interface details for the ESP32-based controller board.

## LILYGO T-Relay Board Specifications

### Board Identification
- **Model**: LILYGO T-Relay (4-Channel Relay Version)
- **Microcontroller**: ESP32-WROOM-32
- **Board Revision**: V1.2 (verify on PCB silkscreen)
- **Operating Voltage**: 3.3V (ESP32) / 12V (Relay Supply)

### Physical Characteristics
- **Dimensions**: 68mm × 53mm × 15mm (L×W×H)
- **Mounting**: 4× M3 mounting holes
- **Connectors**: USB-C programming, screw terminals for I/O
- **LEDs**: Power, WiFi status, relay status indicators

## Power Supply Pins

| Pin | Function | Voltage | Current | Notes |
|-----|----------|---------|---------|--------|
| VIN | Power Input | 7-30V DC | 500mA max | Main power supply input |
| 5V | 5V Output | 5.0V ±0.1V | 500mA max | Regulated 5V output |
| 3V3 | 3.3V Output | 3.3V ±0.1V | 600mA max | ESP32 supply voltage |
| GND | Ground | 0V | - | System ground reference |

### Power Supply Notes
- **Input Protection**: Reverse polarity protection diode
- **Regulation**: Switching regulator (high efficiency)
- **Ripple**: <50mV at full load
- **Startup**: Soft-start circuit prevents inrush current

## Relay Control Outputs

| GPIO | Relay | Terminal | Function | Max Current | Notes |
|------|-------|----------|----------|-------------|--------|
| GPIO5 | Relay 1 | R1 | Main Power | 10A @ 30V DC | Master power control |
| GPIO21 | Relay 2 | R2 | Glow Plugs | 10A @ 30V DC | 20-second timer cycle |
| GPIO22 | Relay 3 | R3 | Starter | 10A @ 30V DC | Momentary activation |
| GPIO18 | Relay 4 | R4 | Lights | 10A @ 30V DC | Combined front/back |

### Relay Specifications
- **Type**: SPDT (Single Pole, Double Throw)
- **Contact Rating**: 10A @ 30V DC / 10A @ 250V AC
- **Coil Voltage**: 12V DC
- **Coil Resistance**: ~150Ω
- **Switching Time**: <10ms
- **Mechanical Life**: >10 million operations
- **Electrical Life**: >100,000 operations at rated load

### Relay Terminal Layout
Each relay has three terminals:
- **COM** (Common): Connected to power source
- **NO** (Normally Open): Connected when relay is energized
- **NC** (Normally Closed): Connected when relay is de-energized

## Analog Input Pins (ADC)

| GPIO | ADC Channel | Function | Input Range | Resolution | Sampling Rate |
|------|-------------|----------|-------------|------------|---------------|
| GPIO36 | ADC1_CH0 | Engine Temp | 0-3.3V | 12-bit | 1kHz |
| GPIO39 | ADC1_CH3 | Oil Pressure | 0-3.3V | 12-bit | 1kHz |
| GPIO34 | ADC1_CH6 | Battery Voltage | 0-3.3V | 12-bit | 1kHz |
| GPIO35 | ADC1_CH7 | Fuel Level | 0-3.3V | 12-bit | 1kHz |

### ADC Specifications
- **Resolution**: 12-bit (4096 counts)
- **Reference Voltage**: 1.1V internal (with attenuation)
- **Attenuation**: 11dB (0-3.3V input range)
- **Accuracy**: ±2% of full scale
- **Input Impedance**: 100MΩ typical
- **Conversion Time**: 100µs typical

### ADC Input Protection
- **ESD Protection**: ±2kV contact discharge
- **Overvoltage**: Clamping diodes to 3.3V and GND
- **Input Filtering**: 100nF ceramic capacitor recommended

## Digital Input Pins

| GPIO | Function | Logic Level | Pull-up/down | Interrupt | Notes |
|------|----------|-------------|--------------|-----------|--------|
| GPIO27 | Alternator Status | 3.3V TTL | Internal pull-up | Yes | Active LOW signal |
| GPIO14 | Engine Run | 3.3V TTL | Internal pull-up | Yes | Active HIGH signal |

### Digital Input Specifications
- **Input High**: >2.0V (logic 1)
- **Input Low**: <0.8V (logic 0)
- **Input Current**: <1µA (high impedance)
- **Pull-up Resistor**: 45kΩ internal (when enabled)
- **Pull-down Resistor**: 45kΩ internal (when enabled)

## Communication Interfaces

### WiFi (Built-in)
- **Standard**: 802.11 b/g/n (2.4GHz)
- **Antenna**: PCB trace antenna (onboard)
- **Range**: 50-100m line of sight
- **Power**: +20dBm maximum transmit power
- **Security**: WPA2/WPA3 supported

### UART (Programming/Debug)
- **USB Connector**: USB-C
- **Baud Rate**: 115200 bps (default)
- **Chip**: CH340 USB-to-Serial converter
- **Pins**: GPIO1 (TX), GPIO3 (RX)

### SPI (Available but not used)
- **MOSI**: GPIO23
- **MISO**: GPIO19
- **SCLK**: GPIO18 (shared with Relay 4)
- **CS**: User defined

### I2C (Available but not used)
- **SDA**: GPIO21 (shared with Relay 2)
- **SCL**: GPIO22 (shared with Relay 3)

## Status LEDs

| LED | Color | Function | GPIO | Notes |
|-----|-------|----------|------|--------|
| PWR | Red | Power On | - | Always on when powered |
| WIFI | Blue | WiFi Status | GPIO2 | Blinks during connection |
| R1 | Green | Relay 1 Status | - | On when relay energized |
| R2 | Green | Relay 2 Status | - | On when relay energized |
| R3 | Green | Relay 3 Status | - | On when relay energized |
| R4 | Green | Relay 4 Status | - | On when relay energized |

## Programming Interface

### Boot Mode Selection
- **GPIO0**: Boot mode select (pulled high for normal boot)
- **EN**: Reset button (active low)
- **Auto-programming**: Supported via CH340 RTS/DTR control

### Flash Memory
- **Size**: 4MB (32Mbit) serial flash
- **Partitions**: Boot loader, application, file system
- **OTA**: Over-the-air updates supported

## Unused/Available Pins

| GPIO | Function | Notes |
|------|----------|--------|
| GPIO0 | Boot Mode | Can be used as input after boot |
| GPIO2 | WiFi LED | Can be used as output |
| GPIO4 | General I/O | Available for expansion |
| GPIO12 | General I/O | Available for expansion |
| GPIO13 | General I/O | Available for expansion |
| GPIO15 | General I/O | Available for expansion |
| GPIO16 | General I/O | Available for expansion |
| GPIO17 | General I/O | Available for expansion |
| GPIO25 | DAC1 | Can be used as analog output |
| GPIO26 | DAC2 | Can be used as analog output |
| GPIO32 | General I/O | Available for expansion |
| GPIO33 | General I/O | Available for expansion |

### Expansion Considerations
- **Current Sourcing**: 12mA per pin maximum
- **Total Current**: 200mA total for all GPIO pins
- **5V Tolerance**: Not 5V tolerant (3.3V maximum)

## Environmental Specifications

### Operating Conditions
- **Temperature Range**: -40°C to +85°C
- **Humidity**: 0-95% relative humidity (non-condensing)
- **Vibration**: 10-500Hz, 2G acceleration
- **Shock**: 30G peak, 11ms duration

### Storage Conditions
- **Temperature Range**: -40°C to +125°C
- **Humidity**: 0-95% relative humidity (non-condensing)
- **Shelf Life**: 10+ years with proper storage

## PCB Layout Considerations

### Ground Planes
- **Digital Ground**: Separate ground plane for ESP32
- **Analog Ground**: Star ground connection for ADC references
- **Power Ground**: Heavy copper pour for relay switching currents

### Power Distribution
- **12V Power**: 2oz copper minimum for relay supply
- **3.3V Power**: Dedicated regulator circuit with filtering
- **Decoupling**: 100nF ceramic capacitors on all power pins

### EMI/RFI Mitigation
- **WiFi Antenna**: Keep clear of high-current traces
- **Relay Switching**: Freewheeling diodes on relay coils
- **Input Filtering**: RC filters on all analog inputs

## Mechanical Specifications

### Mounting Points
- **Hole Size**: 3.2mm diameter (M3 screws)
- **Hole Spacing**: Standard grid pattern
- **Mounting Torque**: 0.5 N⋅m maximum

### Connector Specifications
- **Screw Terminals**: 3.5mm pitch, 12-22 AWG wire range
- **USB Connector**: USB-C, rated for 10,000 insertion cycles
- **Torque**: 0.4 N⋅m for screw terminals

## Electrical Ratings

### Absolute Maximum Ratings
- **Power Supply Voltage**: 35V DC
- **GPIO Voltage**: 3.6V
- **Storage Temperature**: -65°C to +150°C
- **Operating Current**: 1A maximum

### Recommended Operating Conditions
- **Power Supply**: 12V ±10%
- **GPIO Current**: <12mA per pin
- **Operating Temperature**: -20°C to +70°C
- **Ambient Humidity**: <85% RH

## Safety Considerations

### Electrical Safety
- **Isolation**: No galvanic isolation between control and power circuits
- **Fusing**: External fuses required on all relay outputs
- **Overcurrent**: No built-in overcurrent protection on relay contacts

### Installation Safety
- **Disconnect Power**: Always disconnect power before making connections
- **Proper Grounding**: Ensure reliable chassis ground connection
- **Wire Gauge**: Use appropriate wire gauge for expected currents
- **Environmental**: Install in weather-resistant enclosure

## Troubleshooting Guide

### Power Issues
1. **No Power LED**: Check input voltage and polarity
2. **Voltage Drop**: Verify adequate current supply capability
3. **Regulation**: Check for proper 3.3V and 5V output levels

### Relay Issues
1. **No Switching**: Verify GPIO output levels and relay coil continuity
2. **Contact Welding**: Check for overcurrent conditions and proper fusing
3. **Slow Switching**: Verify adequate relay supply voltage

### ADC Issues
1. **Incorrect Readings**: Check voltage divider networks and references
2. **Noisy Readings**: Add input filtering capacitors
3. **Drift**: Verify temperature stability and calibration

### Communication Issues
1. **WiFi Problems**: Check antenna placement and interference sources
2. **Programming Issues**: Verify USB cable and driver installation
3. **Serial Communication**: Check baud rate and cable connections
