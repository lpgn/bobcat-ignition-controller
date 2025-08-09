# Getting Started

## Prerequisites (Windows)

- PlatformIO CLI (use the full path, not `pio`):
  - C:\.platformio\penv\Scripts\platformio.exe
- USB cable (only for initial flashing if needed)

## Build

```powershell
& 'C:\.platformio\penv\Scripts\platformio.exe' run
```

## Build filesystem (LittleFS)

```powershell
& 'C:\.platformio\penv\Scripts\platformio.exe' run --target buildfs
```

## OTA upload options

### First-time: ElegantOTA (visible browser)

- Open http://DEVICE_IP/update
- Select “Firmware” → choose `.pio\build\esp32dev\firmware.bin` → Upload → wait for “Update Successful”
- Select “LittleFS / SPIFFS” → choose `.pio\build\esp32dev\littlefs.bin` → Upload → wait for “Update Successful”
- Keep the browser visible for verification (no headless mode)

### Ongoing updates: PlatformIO CLI (ArduinoOTA)

- Firmware OTA:

```powershell
& 'C:\\.platformio\\penv\\Scripts\\platformio.exe' run -e esp32dev-ota -t upload --upload-port DEVICE_IP
```

- Filesystem OTA:

```powershell
& 'C:\\.platformio\\penv\\Scripts\\platformio.exe' run -e esp32dev-ota -t buildfs
& 'C:\\.platformio\\penv\\Scripts\\platformio.exe' run -e esp32dev-ota -t uploadfs --upload-port DEVICE_IP
```
