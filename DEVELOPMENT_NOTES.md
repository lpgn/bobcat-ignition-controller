# Development Notes for Bobcat Ignition Controller

## Important Commands and Setup Information

### PlatformIO Commands
**CRITICAL: Always use full path to platformio.exe on this system!**

```powershell
# Compile firmware only
C:\.platformio\penv\Scripts\platformio.exe run

# Upload firmware to ESP32
C:\.platformio\penv\Scripts\platformio.exe run --target upload --upload-port COM8

# Build filesystem image
C:\.platformio\penv\Scripts\platformio.exe run --target buildfs

# Upload filesystem image to ESP32
C:\.platformio\penv\Scripts\platformio.exe run --target uploadfs

# Clean build
C:\.platformio\penv\Scripts\platformio.exe run --target clean
```

**DO NOT USE:** `pio run` - This command will fail on this system!

### Hardware Configuration
- **ESP32 Board**: ESP32 Dev Module
- **COM Port**: COM8
- **Wake-Up Pin**: GPIO0 (BOOT button)
- **Target**: esp32dev environment

### Development Status
- **Engine Sensors**: NOT CONNECTED (development override in place)
- **Sleep Mode**: ✅ Implemented and tested with manual override
- **OTA Updates**: ✅ ElegantOTA v3.1.7 integrated at `/update` - Fully tested
- **WiFi**: AP mode "Bobcat-Control" / password "bobcat123"
- **Sensor Calibration**: ✅ Auto-calibration workflow implemented and tested
- **Raw Sensor Data**: ✅ Live ADC readouts available in settings

### Testing Notes
- `isEngineRunning()` returns `false` by default (no sensors connected)
- Sleep mode works without requiring actual engine sensors
- "Sleep Now" button should work immediately after latest fixes

### File Structure
- **Web Files**: `/data/` directory (HTML, CSS, JS)
- **Source Code**: `/src/` directory  
- **Headers**: `/include/` directory
- **Config**: `platformio.ini`

### Known Issues Fixed
- ✅ Sleep mode now bypasses activity timer for manual sleep
- ✅ Engine running detection overridden for development
- ✅ OTA links corrected to relative paths

### Future Tasks
- Connect actual engine sensors and remove development overrides
- Test sleep/wake cycle with real hardware
- Validate all safety systems with connected sensors

## OTA Update Workflow

**Status**: ✅ Tested and Verified Working

The project uses ElegantOTA v3.1.7 for over-the-air firmware and filesystem updates. This allows updating both the ESP32 firmware and the web interface files without physical access to the device.

### Prerequisites
- Device connected to WiFi network
- PlatformIO installed and configured
- Web browser access to device IP address

### Access Methods
1. **Direct URL**: Navigate to `http://device-ip/update`
2. **Via Settings**: Click "🚀 Start OTA Update" button on settings page (`/settings.html`)

### Step-by-Step Update Process

#### 1. Build Update Files
```powershell
# Navigate to project directory
cd C:\Users\lpgn\gits_go_here\bobcat-ignition-controller

# Build firmware binary
C:\.platformio\penv\Scripts\platformio.exe run

# Build filesystem binary (contains web interface)
C:\.platformio\penv\Scripts\platformio.exe run --target buildfs
```

This creates two files:
- **Firmware**: `.pio\build\esp32dev\firmware.bin` (~1-2MB)
- **Filesystem**: `.pio\build\esp32dev\littlefs.bin` (~200-400KB)

#### 2. Access OTA Interface
1. Connect to device at `http://device-ip/update`
2. You'll see the ElegantOTA interface with mode selection dropdown

#### 3. Update Firmware
1. Select "Firmware" from the dropdown menu
2. Click "Choose File" button
3. Navigate to and select `.pio\build\esp32dev\firmware.bin`
4. Click "Update" button
5. Wait for upload progress bar (typically 30-60 seconds)
6. Device will automatically restart after successful update

#### 4. Update Filesystem (Web Interface)
1. After firmware update completes, return to `/update` page
2. Select "LittleFS / SPIFFS" from the dropdown menu
3. Click "Choose File" button  
4. Navigate to and select `.pio\build\esp32dev\littlefs.bin`
5. Click "Update" button
6. Wait for upload progress bar (typically 10-20 seconds)
7. Device will restart again automatically

#### 5. Verification
- Navigate to main interface (`http://device-ip/`)
- Check settings page (`/settings.html`) for new features
- Verify all functionality works as expected

### Important Notes

**Update Order**: Always update firmware first, then filesystem. This ensures compatibility between backend and frontend code.

**Restart Behavior**: Device restarts automatically after each update. Wait ~10-15 seconds before accessing the interface again.

**Rollback**: No automatic rollback mechanism - ensure you have working binaries before updating production devices.

**Safety**: Never power off the device during upload. Interruption can brick the device requiring serial recovery.

**Network**: Ensure stable WiFi connection during updates. Poor connection can cause upload failures.

**Validation**: Both binaries are MD5 verified during upload to prevent corruption.

### Troubleshooting

**Upload Failed**: 
- Check file size limits (firmware <4MB, filesystem <2MB)
- Verify stable network connection  
- Try updating one component at a time
- Check serial monitor for error messages

**Device Not Responding**: 
- Wait 30+ seconds for full restart sequence
- Check serial monitor for boot messages
- Power cycle device if necessary

**Web Interface Issues After Update**:
- Clear browser cache completely
- Try hard refresh (Ctrl+F5)
- Verify filesystem was uploaded correctly
- Check console for JavaScript errors

### Sensor Calibration After Updates

After updating firmware, recalibrate sensors if needed:
1. Navigate to Settings → Raw Sensor Data & Calibration section
2. Use actual multimeter readings for battery voltage reference
3. Enter real measurements in "Actual" fields
4. Click "🎯 Auto-Calibrate from Actual Values" button
5. Restart device to apply new calibration constants

---
*Last Updated: August 6, 2025*
*This file serves as a reference for development commands and project status*
