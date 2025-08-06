# Bobcat Ignition Controller - OTA Update Scripts

This directory contains automated scripts for performing Over-The-Air (OTA) updates on your Bobcat Ignition Controller device. The scripts follow the development workflow documented in `DEVELOPMENT_NOTES.md`.

## Available Scripts

### 1. Node.js/Playwright Script (Fully Automated)
- **File**: `ota_update_script.js`
- **Description**: Fully automated browser-driven OTA updates
- **Requirements**: Node.js, Playwright
- **Usage**: `npm run ota-update`

### 2. PowerShell Script (Semi-Automated)
- **File**: `ota_update.ps1`
- **Description**: Compiles binaries automatically, guides through manual upload
- **Requirements**: PowerShell, PlatformIO
- **Usage**: `.\ota_update.ps1` or double-click `ota_update.bat`

### 3. Batch File (Windows Wrapper)
- **File**: `ota_update.bat`
- **Description**: Simple wrapper for PowerShell script
- **Requirements**: Windows, PowerShell
- **Usage**: Double-click the file

## Quick Start

### Option A: Fully Automated (Recommended)

1. **Install dependencies:**
   ```powershell
   npm install
   npm run install-playwright
   ```

2. **Run automated update:**
   ```powershell
   npm run ota-update
   ```

### Option B: Semi-Automated (Easier Setup)

1. **Double-click:** `ota_update.bat`
2. **Follow prompts** for manual upload steps

## Script Features

### What the Scripts Do:

1. **✅ Compile Firmware**
   - Uses correct PlatformIO path: `C:\.platformio\penv\Scripts\platformio.exe run`
   - Builds `firmware.bin` in `.pio\build\esp32dev\`

2. **✅ Upload Firmware**
   - Connects to device at `http://192.168.1.128/update`
   - Selects "Firmware" mode
   - Uploads `firmware.bin` via ElegantOTA
   - Waits for "Update Successful" confirmation

3. **✅ Compile Filesystem**
   - Uses: `C:\.platformio\penv\Scripts\platformio.exe run --target buildfs`
   - Builds `littlefs.bin` with web interface files

4. **✅ Upload Filesystem**
   - Switches to "LittleFS / SPIFFS" mode
   - Uploads `littlefs.bin` via ElegantOTA
   - Waits for completion and device restart

5. **✅ Verification**
   - Tests device responsiveness after restart
   - Verifies main interface and settings page load

## Configuration

### Default Settings:
- **Device IP**: `192.168.1.128`
- **Project Path**: `c:\Users\lpgn\gits_go_here\bobcat-ignition-controller`
- **PlatformIO Path**: `C:\.platformio\penv\Scripts\platformio.exe`

### Customizing Settings:

**Node.js Script**: Edit the `CONFIG` object in `ota_update_script.js`
```javascript
const CONFIG = {
    deviceIP: 'http://192.168.1.128',
    projectPath: 'c:\\Users\\lpgn\\gits_go_here\\bobcat-ignition-controller',
    // ... other settings
};
```

**PowerShell Script**: Use parameters
```powershell
.\ota_update.ps1 -DeviceIP "192.168.1.100" -ProjectPath "C:\MyProject"
```

## Prerequisites

### For All Scripts:
- PlatformIO installed at `C:\.platformio\penv\Scripts\platformio.exe`
- Device accessible at specified IP address
- Device connected to same network as development machine

### For Node.js Script:
- Node.js (v14 or higher)
- Playwright package

### For PowerShell Script:
- Windows PowerShell 5.1 or PowerShell Core
- Web browser (for manual upload steps)

## Troubleshooting

### Common Issues:

**"PlatformIO not found"**
- Verify PlatformIO installation path
- Install PlatformIO if missing: [Installation Guide](https://platformio.org/install)

**"Device not responding"**
- Check device IP address
- Ensure device is powered on and connected to WiFi
- Try accessing `http://192.168.1.128/` manually

**"Upload failed"**
- Ensure stable WiFi connection
- Check file sizes (firmware <4MB, filesystem <2MB)
- Restart device and try again

**"Compilation failed"**
- Check project directory path
- Ensure `platformio.ini` exists in project root
- Review PlatformIO error messages

### Browser Issues (Node.js Script):
```bash
# Reinstall Playwright browsers
npm run install-playwright
```

### PowerShell Execution Policy:
```powershell
# If script won't run, temporarily allow execution
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope CurrentUser
```

## Script Output Examples

### Successful Run:
```
🎯 Starting automated OTA update process...
📍 Target device: http://192.168.1.128
📁 Project path: c:\Users\lpgn\gits_go_here\bobcat-ignition-controller
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

📦 STEP 1: Compiling firmware...
✅ Firmware compilation successful

🚀 STEP 2: Uploading firmware via OTA...
✅ Connected to ElegantOTA interface
📝 Firmware mode selected
📁 Uploading: c:\Users\lpgn\...\firmware.bin
⏳ Uploading firmware... (this may take 1-2 minutes)
✅ Firmware upload successful!

📦 STEP 3: Compiling filesystem...
✅ Filesystem compilation successful

🚀 STEP 4: Uploading filesystem via OTA...
📝 Filesystem mode selected
📁 Uploading: c:\Users\lpgn\...\littlefs.bin
⏳ Uploading filesystem... (this may take 30-60 seconds)
✅ Filesystem upload successful!

🔍 STEP 5: Verifying update...
✅ Update verification successful!

🎉 OTA UPDATE COMPLETED SUCCESSFULLY!
✅ Firmware updated
✅ Filesystem updated
✅ Device verified and responsive
🌐 Access your device at: http://192.168.1.128
```

## Additional npm Scripts

```json
{
  "ota-update": "node ota_update_script.js",           // Full update
  "install-playwright": "npx playwright install chromium", // Install browser
  "firmware-only": "...",                              // Firmware only
  "filesystem-only": "..."                             // Filesystem only
}
```

## Safety Notes

⚠️ **Important:**
- Never power off device during upload
- Ensure stable network connection
- Keep backup binaries for rollback
- Test updates on development device first

## Version History

- **v1.0.0**: Initial automated OTA update scripts
- Supports firmware and filesystem updates
- Compatible with ElegantOTA v3.1.7
- Windows PowerShell and Node.js versions

---

*Last Updated: August 6, 2025*
*Part of Bobcat Ignition Controller project*
