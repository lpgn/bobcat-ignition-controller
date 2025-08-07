# Copilot Instructions - Bobcat Ignition Controller

## Repository Overview

This is an **ESP32-based ignition controller** for old Bobcat equipment (specifically Bobcat 743 with Kubota V1702-BA diesel engine). The system provides web-based control for glow plug preheating, starter control, lighting, and safety monitoring for diesel engines that cannot be stopped electronically.

**⚠️ SAFETY CRITICAL SYSTEM ⚠️** - This code controls high-current automotive relays and diesel engine systems. Incorrect changes can cause equipment damage, safety hazards, or engine damage.

### Key Facts
- **Project Type**: Embedded firmware for ESP32 microcontroller
- **Hardware**: LILYGO T-Relay ESP32 board with 4-channel relay control
- **Target**: Bobcat 743 with Kubota V1702-BA diesel engine (naturally aspirated, mechanical injection)
- **Codebase Size**: ~1,840 lines across 13 C++ files (7 .cpp + 6 .h)
- **Documentation**: 13 markdown files with extensive hardware and wiring details
- **Framework**: Arduino Framework with PlatformIO build system
- **Languages**: C++ (Arduino), HTML/CSS/JavaScript (web interface)

## Build Instructions & Environment Setup

### Prerequisites - ALWAYS Required
1. **PlatformIO CLI** - Install via `pip install platformio`
2. **Internet connection** - Required for first build to download ESP32 platform and libraries
3. **USB cable** - For programming ESP32 (if doing actual hardware testing)

### Build Commands - Validated Working Sequence

```bash
# 1. ALWAYS navigate to project root first
cd /path/to/bobcat-ignition-controller

# 2. Build firmware (requires internet on first run)
pio run

# 3. Optional: Upload to hardware (requires connected ESP32)
pio run --target upload

# 4. Optional: Monitor serial output
pio device monitor --baud 115200
```

### Build Process Details
- **Platform**: espressif32 (downloads ~300MB on first build)
- **Board**: esp32dev (ESP32-WROOM-32 compatible)
- **Framework**: Arduino with LittleFS filesystem
- **Libraries**: ArduinoJson, ESPAsyncWebServer, AsyncTCP, ElegantOTA (auto-downloaded)
- **Build time**: 2-5 minutes first time, 30-60 seconds subsequent builds

### Common Build Issues & Solutions

1. **HTTPClientError during build**
   - **Cause**: No internet connection for platform/library downloads
   - **Solution**: Ensure internet connectivity and retry `pio run`

2. **Platform installation fails**
   - **Cause**: PlatformIO cache corruption
   - **Solution**: `pio platform uninstall espressif32 && pio run`

3. **Library dependency errors**
   - **Cause**: Version conflicts or incomplete downloads
   - **Solution**: `pio lib uninstall --all && pio run`

4. **Upload fails**
   - **Cause**: ESP32 not connected or wrong port
   - **Solution**: Check hardware connection, try `pio device list`

### Validation Steps - No Automated Testing Available
⚠️ **This project has NO unit tests, integration tests, or CI/CD pipeline.**

**Manual validation only:**
1. **Compile check**: `pio run` must complete without errors
2. **Serial monitor**: Check for startup messages if hardware available
3. **Web interface**: Verify web files in `/data/` are valid HTML/CSS/JS
4. **Code review**: Manual review for GPIO pin conflicts and timing issues

## Project Architecture & Layout

### Directory Structure
```
├── src/                    # Main C++ source files (7 files)
│   ├── main.cpp           # Entry point & main loop
│   ├── config.cpp         # Pin definitions & constants
│   ├── hardware.cpp       # GPIO control & relay management
│   ├── safety.cpp         # Safety monitoring & alerts
│   ├── system_state.cpp   # State machine logic
│   ├── settings.cpp       # EEPROM settings management
│   └── web_interface.cpp  # AsyncWebServer & HTTP handlers
├── include/               # Header files (6 files)
│   ├── config.h          # Pin mappings & timing constants
│   ├── hardware.h        # Relay & sensor function declarations
│   ├── safety.h          # Safety threshold definitions
│   ├── system_state.h    # State enum & global variables
│   ├── settings.h        # Settings structure definitions
│   └── web_interface.h   # Web server function declarations
├── data/                 # Web interface files (LittleFS)
│   ├── index.html        # Main control interface
│   ├── settings.html     # Configuration page
│   ├── style.css         # Responsive CSS
│   ├── script.js         # JavaScript control logic
│   └── settings.js       # Settings page JavaScript
├── docs/                 # Extensive documentation (13 files)
├── platformio.ini        # Build configuration & dependencies
└── README.md            # Project overview & pin assignments
```

### Critical Files to Understand Before Making Changes

1. **`include/config.h`** - GPIO pin assignments, timing constants, safety thresholds
2. **`src/main.cpp`** - Main program flow and state machine
3. **`src/hardware.cpp`** - Direct GPIO control and relay operations
4. **`src/safety.cpp`** - Safety monitoring and engine protection logic
5. **`platformio.ini`** - Build configuration and library dependencies

### Hardware Pin Mapping (LILYGO T-Relay Board)
```cpp
// CRITICAL RELAY OUTPUTS (Active HIGH)
const int MAIN_POWER_PIN = 21;    // Main Power Relay (Relay 1)
const int GLOW_PLUGS_PIN = 19;    // Glow Plug Relay (Relay 2) 
const int STARTER_PIN = 18;       // Starter Solenoid (Relay 3)
const int LIGHTS_PIN = 5;         // Work Lights (Relay 4)

// ANALOG SENSOR INPUTS (12-bit ADC)
const int ENGINE_TEMP_PIN = 39;    // NTC thermistor
const int OIL_PRESSURE_PIN = 35;   // Resistive sender
const int BATTERY_VOLTAGE_PIN = 36; // Voltage divider
const int FUEL_LEVEL_PIN = 34;     // Float sender

// DIGITAL STATUS INPUTS
const int ALTERNATOR_CHARGE_PIN = 22;    // Charge indicator
const int ENGINE_RUN_FEEDBACK_PIN = 26;  // Engine running feedback
```

### State Machine Logic
The system operates as a real ignition key with these states:
- **OFF** - All systems disabled
- **ON** - Electrical systems active, monitoring enabled
- **GLOW_PLUG** - 20-second glow plug preheat cycle
- **START** - Momentary starter engagement (max 5 seconds)
- **RUNNING** - Engine running, full monitoring active

### Safety Interlocks & Constraints
⚠️ **Engine cannot be stopped electronically** - uses manual fuel lever only
- Low battery protection: < 10.5V prevents start
- Starter timeout: 5-second maximum engagement
- Oil pressure monitoring: Alerts only, no auto-shutdown
- Temperature monitoring: Warning thresholds, manual action required
- Emergency stop: Immediate relay shutdown on abort button

## Development Guidelines for Agents

### What You CAN Safely Modify
- Web interface files in `/data/` (HTML, CSS, JavaScript)
- Safety threshold values in `config.h` (within reasonable limits)
- Timing constants (glow plug duration, timeouts)
- Sensor calibration values and curves
- Serial debugging messages and status reporting
- Settings management and EEPROM storage

### What You MUST NOT Modify Without Expert Review
- GPIO pin assignments in `config.h` (hardware-specific)
- Relay control logic in `hardware.cpp` (safety-critical)
- State machine transitions in `system_state.cpp`
- Safety interlock logic in `safety.cpp`
- Power-on initialization sequences
- Interrupt handlers or timing-critical code

### Dependencies & External Requirements
- **Arduino Framework** - ESP32 Arduino Core ~2.0+
- **AsyncWebServer** - Web interface requires exact version compatibility
- **ArduinoJson** - Settings serialization/deserialization
- **ElegantOTA** - Over-the-air firmware updates
- **LittleFS** - File system for web interface storage

### Hardware Dependencies
- **ESP32-WROOM-32** - Specific GPIO capabilities required
- **LILYGO T-Relay** - 4-channel relay board with exact pin mapping
- **12V automotive power** - Voltage ranges and transient protection
- **Specific sensors** - NTC thermistor, resistive senders with known curves
- **Automotive relays** - 10A+ capacity for glow plugs and starter

### Documentation Resources (Read Before Coding)
- **`docs/hardware_overview.md`** - Complete hardware specifications
- **`docs/wiring_guide.md`** - Installation and connection details
- **`docs/gpio_connections.md`** - Pin assignments and interface specs
- **`docs/system_control_diagram.md`** - State machine and control logic
- **`docs/board_development.md`** - Detailed pin-by-pin analysis
- **`README.md`** - Quick overview and basic pin mappings

### Debugging & Troubleshooting
- **Serial Monitor**: Connect at 115200 baud for detailed logs
- **Hardware Required**: Real ESP32 board needed for meaningful testing
- **No Simulation**: Cannot test relay operations without physical hardware
- **Safety Testing**: All safety features require actual sensors and conditions

### Performance Constraints
- **Real-time Requirements**: 10ms main loop for responsive control
- **Memory Limits**: ESP32 has limited RAM, avoid large buffers
- **Power Consumption**: System must operate on 12V automotive power
- **Temperature Range**: -40°C to +85°C automotive environment
- **EMI Immunity**: Must work in high-EMI automotive environment

## Emergency Actions & Safety Notes

### If Code Changes Cause Issues
1. **Immediate**: Disconnect battery to ESP32 board
2. **Hardware**: Manually check all relay positions (should be OFF)
3. **Recovery**: Flash known-good firmware via `pio run --target upload`
4. **Testing**: Always test changes on bench before installing in vehicle

### Critical Safety Reminders
- **High Current Relays**: 40-100A switching capacity - respect current limits
- **Automotive Voltages**: 12V system with possible 40V transients
- **Diesel Engine**: Glow plugs draw 60A+ total current during preheat
- **No Auto-Shutdown**: Engine runs independently once started
- **Manual Intervention**: All emergency stops require manual action

---

**Trust these instructions** - They are based on thorough repository analysis and testing. Only search further if information is incomplete or incorrect. The extensive documentation in `/docs/` contains additional technical details if needed.