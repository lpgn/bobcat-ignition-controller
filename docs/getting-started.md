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

## Upload via ElegantOTA (visible browser)

- Open http://<device-ip>/update
- Select “Firmware” → choose .pio\build\esp32dev\firmware.bin → Upload → wait for “Update Successful”
- Select “LittleFS / SPIFFS” → choose .pio\build\esp32dev\littlefs.bin → Upload → wait for “Update Successful”
- Never use headless mode; keep the browser visible for verification
