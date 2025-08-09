# Bobcat Ignition Controller

ESP32-based control system for managing old Bobcat equipment with web-based interface, glow plug preheating, and safety monitoring.

## Documentation

Single source for build/flash/OTA commands: [.github/copilot-instructions.md](.github/copilot-instructions.md)

Minimal docs:

- Manual: [docs/manual.md](docs/manual.md)
- Hardware: [docs/hardware.md](docs/hardware.md)
- Software: [docs/software.md](docs/software.md)
- Build & Deploy: [docs/build-and-deploy.md](docs/build-and-deploy.md)
- Schematics: [docs/schematics.mmd](docs/schematics.mmd)

## OTA updates (summary)

- First-time upload: use ElegantOTA in a visible browser at `http://DEVICE_IP/update` to upload both firmware and LittleFS images.
- Ongoing updates: use PlatformIO CLI with ArduinoOTA (espota). See commands in [.github/copilot-instructions.md](.github/copilot-instructions.md).

## Features

- **Web Interface**: Modern responsive web interface with grouped controls
- **Power Management**: Main power relay control for all electrical systems
- **Glow Plug Control**: Automatic 20-second preheat cycle before engine start
- **Starter Control**: Manages starter motor engagement with timeout protection
- **Light Controls**: Independent front and back light relay control
- **Safety Monitoring**: Engine temperature and oil pressure monitoring with visual alerts
- **Override Function**: Emergency start bypass for critical situations
- **Alert System**: Visual web interface alerts for safety conditions

## Important Notes

⚠️ **This system is designed for older Bobcat equipment where:**

- Engine cannot be stopped electronically (manual lever only)
- No ignition/run relay is present (engine runs independently once started)
- Safety system provides alerts only - cannot auto-shutdown engine

## Hardware Requirements

### ESP32 Development Board

- ESP32-WROOM-32 or compatible
- Minimum 30 GPIO pins recommended

### Relay Modules

- 2x High-current automotive relays (30A minimum)
- Relay driver modules (optoisolated recommended)

### Sensors

- Engine temperature sensor (NTC thermistor or similar)
- Oil pressure sensor (0-5V output)
- Battery voltage monitoring (voltage divider circuit)

### User Interface

- Momentary push buttons (2x - Start/Stop)
- Status LED
- Buzzer/alarm (optional)

## Pin Configuration

| Function | GPIO Pin | Type | Description |
|----------|----------|------|-------------|
| Glow Plug Relay | 2 | Output | Controls glow plug relay |
| Ignition Relay | 4 | Output | Controls starter motor relay |
| Start Button | 18 | Input (Pullup) | Engine start button |
| Stop Button | 19 | Input (Pullup) | Engine stop/abort button |
| Status LED | 5 | Output | System status indicator |
| Buzzer | 21 | Output | Alarm/notification buzzer |
| Engine Temp | 34 | ADC Input | Engine temperature sensor |
| Oil Pressure | 35 | ADC Input | Oil pressure sensor |
| Battery Voltage | 32 | ADC Input | Battery voltage monitor |

## Operation Sequence

1. **IDLE**: System ready, waiting for start command
2. **GLOW PLUG HEATING**: 20-second preheat cycle
3. **READY TO START**: Glow plugs heated, ready for ignition
4. **STARTING**: Starter motor engaged (max 5 seconds)
5. **RUNNING**: Engine running, monitoring systems active
6. **SHUTDOWN**: Safe shutdown sequence

## Safety Features

- **Low Battery Protection**: Prevents start if battery voltage < 10.5V
- **Ignition Timeout**: Limits starter engagement to 5 seconds maximum
- **Emergency Stop**: Immediate shutdown on stop button press
- **System Monitoring**: Continuous monitoring of critical parameters
- **Relay Protection**: All relays default to OFF state on power-up

## Installation

### PlatformIO

```pwsh
cd bobcat-ignition-controller
pio run --target upload
```

### Arduino IDE

1. Install ESP32 board package
2. Open `src/main.cpp` in Arduino IDE
3. Select ESP32 Dev Module board
4. Upload to ESP32

## Wiring Diagram

```text
ESP32          Relay Module       Bobcat System
GPIO2  ----→   Relay 1 Control ----→ Glow Plug Circuit
GPIO4  ----→   Relay 2 Control ----→ Starter Motor Circuit

ESP32          User Interface
GPIO18 ←----   Start Button (Pullup)
GPIO19 ←----   Stop Button (Pullup)
GPIO5  ----→   Status LED
GPIO21 ----→   Buzzer

ESP32          Sensors
GPIO34 ←----   Engine Temperature (via voltage divider)
GPIO35 ←----   Oil Pressure Sensor
GPIO32 ←----   Battery Voltage (via voltage divider)
```

## Configuration

### Timing Constants

- `GLOW_PLUG_DURATION`: Glow plug heating time (default: 20 seconds)
- `IGNITION_TIMEOUT`: Maximum starter engagement time (default: 5 seconds)
- `DEBOUNCE_DELAY`: Button debounce time (default: 50ms)

### Safety Thresholds

- Battery voltage minimum: 10.5V
- Engine temperature limits: TBD based on sensor specifications
- Oil pressure limits: TBD based on engine specifications

## Status LED Patterns

- **Slow Blink (2s)**: IDLE - Ready for start
- **Medium Blink (0.5s)**: Glow plug heating
- **Solid ON**: Ready to start or Running
- **Fast Blink (0.1s)**: Starting/Cranking
- **Very Fast Blink (0.2s)**: Error condition

## Troubleshooting

### Common Issues

1. **Won't Start**: Check battery voltage, verify relay connections
2. **Glow Plugs Not Heating**: Check relay 1 wiring and fuse
3. **Starter Won't Engage**: Check relay 2 wiring and connections
4. **False Error Conditions**: Verify sensor wiring and calibration

### Serial Monitor

Connect to ESP32 at 115200 baud for detailed operation logs and debugging information.

## Safety Warnings

⚠️ **HIGH VOLTAGE/CURRENT SYSTEM** ⚠️

- This system controls high-current automotive circuits
- Improper installation can damage equipment or cause injury
- Always disconnect battery before installation
- Use appropriate fuses and circuit protection
- Test all safety systems before first use

## License

This project is provided as-is for educational and development purposes.

## Contributing

This is a custom Bobcat ignition controller project. Contributions and improvements welcome.

## Version History

- v1.0 - Initial implementation with basic ignition sequence control
