# Build and Deploy (PlatformIO + OTA)

## Build firmware

```powershell
platformio run
```

## Build filesystem

```powershell
platformio run --target buildfs
```

## OTA options

There are two OTA paths available. Use ElegantOTA once to seed the device, then prefer PlatformIO CLI (ArduinoOTA) for fast, scriptable updates.

### 1) First-time seed: ElegantOTA (manual, visible)

1. Visit: http://DEVICE_IP/update
2. Firmware: upload `.pio\build\esp32dev\firmware.bin` → wait for “Update Successful”
3. LittleFS/SPIFFS: upload `.pio\build\esp32dev\littlefs.bin` → wait for “Update Successful”
4. Wait 10–20s for automatic reboot after firmware upload
5. Refresh UI (Ctrl+F5) to bypass cache

Notes:
- Transient 192.168.4.1 fetch errors during reboot are expected; reload after ~10–20s.


### 2) Ongoing updates: PlatformIO CLI (ArduinoOTA/espota)

Requirements: device on LAN, ArduinoOTA enabled in firmware (already configured), TCP port 3232 open.

- Firmware OTA:

```powershell
platformio run -e esp32dev-ota -t upload --upload-port <DEVICE_IP>
```

- Filesystem OTA (LittleFS):

```powershell
platformio run -e esp32dev-ota -t buildfs
platformio run -e esp32dev-ota -t uploadfs --upload-port <DEVICE_IP>
```

Optional: if you set an OTA password in code, append: `--auth=<PASSWORD>`
