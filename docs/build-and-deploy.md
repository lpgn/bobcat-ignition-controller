# Build and Deploy (PlatformIO + ElegantOTA)

## Build firmware

```powershell
& 'C:\.platformio\penv\Scripts\platformio.exe' run
```

## Build filesystem

```powershell
& 'C:\.platformio\penv\Scripts\platformio.exe' run --target buildfs
```

## ElegantOTA upload (manual, visible)

1. Visit http://DEVICE_IP/update
2. Firmware tab → upload .pio\build\esp32dev\firmware.bin → wait for success
3. LittleFS/SPIFFS tab → upload .pio\build\esp32dev\littlefs.bin → wait for success
4. Give the device 10–20s to reboot after firmware upload
5. Refresh the UI (Ctrl+F5) to bypass cache

Notes:
- For production, always use ElegantOTA (not serial upload).
- If you see transient 192.168.4.1 fetch errors during reboots, ignore; retry after 10–20s.
