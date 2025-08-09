# Copilot Instructions - Bobcat Ignition Controller

## Repository Overview

This is an **ESP32-based ignition controller** for old Bobcat equipment (specifically Bobcat 743 with Kubota V1702-BA diesel engine). The system provides web-based control for glow plug preheating, starter control, lighting, and safety monitoring for diesel engines that cannot be stopped electronically.

**🚨 CRITICAL SAFETY ALERT 🚨**
**CURRENT DESIGN HAS SAFETY VIOLATIONS - SEE [docs/SAFETY_CRITICAL.md](docs/SAFETY_CRITICAL.md)**

- **❌ GLOW PLUG RELAY**: 40-80A load exceeds 10A relay rating - **FIRE/EXPLOSION HAZARD**
- **⚠️ LIGHTING RELAY**: 9.2A load with insufficient safety margin
- **❌ MISSING INTERLOCKS**: No seat bar or neutral safety switches

**⚠️ SAFETY CRITICAL SYSTEM ⚠️** - This code controls high-current automotive relays and diesel engine systems. The current implementation has **MANDATORY** safety corrections required before use.

### Key Facts
- **Project Type**: Embedded firmware for ESP32 microcontroller
- **Hardware**: LILYGO T-Relay ESP32 board with 4-channel relay control
- **Target**: Bobcat 743 with Kubota V1702-BA diesel engine (naturally aspirated, mechanical injection)
- **Framework**: Arduino Framework with PlatformIO build system
- **Languages**: C++ (Arduino), HTML/CSS/JavaScript (web interface)

## Build Instructions & Environment Setup

### Prerequisites - ALWAYS Required
1. **PlatformIO CLI** - Must be installed and available in PATH
   - Command must work: `platformio run`
   - If command not found, PlatformIO installation is required

2. **USB cable** - For programming ESP32 (if doing actual hardware testing)

### One-Time Setup: Add PlatformIO to PATH Permanently (Windows)

If `platformio` command is not recognized, add it to your system PATH permanently:

```powershell
# Add PlatformIO to user PATH permanently (run once)
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\.platformio\penv\Scripts", [EnvironmentVariableTarget]::User)

# Verify it works (restart terminal if needed)
platformio --version
```

### Build Commands - SIMPLE APPROACH ONLY

**CRITICAL: Only use these simple commands. No complex PATH manipulation in tasks.**

```powershell
# Build firmware
platformio run

# Build filesystem (LittleFS)
platformio run --target buildfs
```

**If `platformio` command is not found, use the PATH setup above, then restart terminal. Do NOT use complex task workarounds.**

### Upload options

```bash
# Option A) Playwright MCP + ElegantOTA (recommended for remote updates)
mcp_playwright_browser_navigate: http://192.168.1.128/update
mcp_playwright_browser_select_option: "Firmware"
mcp_playwright_browser_file_upload: .pio/build/esp32dev/firmware.bin
mcp_playwright_browser_wait_for: "Update Successful"

# 2. Filesystem Update:
mcp_playwright_browser_select_option: "LittleFS / SPIFFS"
mcp_playwright_browser_file_upload: .pio/build/esp32dev/littlefs.bin
mcp_playwright_browser_wait_for: "Update Successful"

# Option B) PlatformIO OTA Upload (command line)
# Firmware OTA (requires device IP)
platformio run -t upload --upload-port 192.168.1.128

# Filesystem OTA
platformio run -t uploadfs --upload-port 192.168.1.128

# Option C) Serial Upload (first-time flashing or recovery)
platformio run -t upload
platformio run -t uploadfs
```

### Validation Steps - No Automated Testing Available

**Manual validation only:**
1. **Compile check**: `platformio run` must complete without errors
2. **Upload check**: Verify firmware and filesystem uploads via Playwright logs
3. **Web interface**: Verify web files in `/data/` are valid HTML/CSS/JS

- **Playwright Visibility**: Run Playwright in **NON-HEADLESS MODE** so user can see browser actions
- **Never use headless mode** - user must see what Playwright is doing for debugging and verification

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
2. **`src/main.cpp`** - Main program flow and state machine (keep this file clear and concise, all logic resides in other files)
3. **`src/hardware.cpp`** - Direct GPIO control and relay operations
4. **`src/safety.cpp`** - Safety monitoring and engine protection logic
5. **`platformio.ini`** - Build configuration and library dependencies

### Development Rules
- **Never use delay()** - Use millis() instead as delay() causes blocking
- **Keep logic in C++** - Web interface files should only handle UI, all business logic in C++ backend

### Hardware Pin Mapping (LILYGO T-Relay Board)
```cpp
// CRITICAL RELAY OUTPUTS (Active HIGH)
const int MAIN_POWER_PIN = 21;    // Main Power Relay (Relay 1)
const int GLOW_PLUGS_PIN = 19;    // Glow Plug Relay (Relay 2) 
const int STARTER_PIN = 18;       // Starter Solenoid (Relay 3)
const int LIGHTS_PIN = 5;         // Work Lights (Relay 4)

// ANALOG SENSOR INPUTS (12-bit ADC)
const int ENGINE_TEMP_PIN = 39;    // NTC thermistor
const int OIL_PRESSURE_PIN = 34;   // Resistive sender
const int BATTERY_VOLTAGE_PIN = 36; // Voltage divider
const int FUEL_LEVEL_PIN = 35;     // Float sender

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

### **CRITICAL RULE: Logic Location**
⚠️ **ALL business logic, calculations, calibrations, and data processing MUST be implemented in C++ backend endpoints, NOT in JavaScript frontend.**

**JavaScript should ONLY be used for:**
- UI interactions and DOM manipulation
- Form validation (basic only)
- Making fetch() calls to C++ API endpoints
- Simple UI state management

**C++ backend MUST handle:**
- All sensor calibration calculations
- Settings validation and processing
- Data transformations and conversions
- Business logic and state management
- Safety checks and interlocks

### What You CAN Safely Modify
- Web interface files in `/data/` (HTML, CSS, JavaScript) - **BUT keep logic in C++**
- Safety threshold values in `config.h` (within reasonable limits)
- Timing constants (glow plug duration, timeouts)
- Sensor calibration values and curves - **IN C++ ONLY**
- Serial debugging messages and status reporting
- Settings management and EEPROM storage - **Backend logic only**
- REST API endpoints in `web_interface.cpp` for business logic

### What You MUST NOT Modify Without Expert Review
- GPIO pin assignments in `config.h` (hardware-specific)
- Relay control logic in `hardware.cpp` (safety-critical)
- State machine transitions in `system_state.cpp`
- Safety interlock logic in `safety.cpp`
- Power-on initialization sequences
- Interrupt handlers or timing-critical code

## Common Development Issues & Solutions

### Settings & Calibration Issues
- **Settings not applied**: Check if `loadCalibrationConstants()` is called after settings changes
- **Calibration sync**: Runtime calibration variables in `hardware.cpp` are loaded from SettingsManager
- **Persistence**: All settings are stored in ESP32 NVS (non-volatile storage) and survive reboots
- **Global access**: Use `g_settingsManager` (declared in `settings.cpp`, extern in `settings.h`)

### Build & Upload Issues  
- **Build failures**: Ensure PlatformIO is in PATH, check platformio.ini for dependencies
- **Web changes not visible**: Run `buildfs` + upload via ElegantOTA web interface
- **Firmware upload**: NEVER use serial upload for production - always use ElegantOTA web interface at `/update`
- **Serial connection failed**: Ignore serial upload errors for production - use OTA workflow instead

### Development Preferences
- **UI Style**: Mobile-first, professional, robust CSS with proper gauge layout
- **Code Quality**: Clean, well-documented, proper error handling
- **Deployment**: Always test builds and provide deployment commands
- **Branding**: Include Fab Farm logo attribution - developed by Fab Farm team
- **Architecture**: RESTful API endpoints in `web_interface.cpp` for all operations
- **Safety System**: Multiple safety checks prevent unsafe engine operations

---

**Trust these instructions** - They are based on thorough repository analysis and testing. Only search further if information is incomplete or incorrect. The extensive documentation in `/docs/` contains additional technical details if needed.