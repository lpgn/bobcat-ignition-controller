# Troubleshooting

## Build fails

- Use the full PlatformIO path:

```powershell
& 'C:\.platformio\penv\Scripts\platformio.exe' run
```

- Check platformio.ini libraries match:
  - ArduinoJson ^6.19.4
  - ESPAsyncWebServer ^3.7.3
  - AsyncTCP ^3.3.7
  - ElegantOTA ^3.1.6

## OTA issues

- Ensure device IP is correct and reachable
- Keep browser visible (no headless)
- Wait 10–20s after firmware OTA
- Hard-refresh to bypass cache

## UI shows “Checking…”

- Backend endpoint might be missing/renamed; verify /status is reachable
- Look at the browser console network tab for failing requests
