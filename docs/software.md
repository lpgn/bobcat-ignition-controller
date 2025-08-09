# Software

- Framework: Arduino (ESP32) on PlatformIO
- Key modules:
  - src/hardware.cpp: GPIO and relay control
  - src/system_state.cpp: state machine
  - src/safety.cpp: safety checks
  - src/web_interface.cpp: AsyncWebServer + ElegantOTA; ArduinoOTA enabled in main.cpp
  - include/config.h: pins and timing constants

Rules:

- Avoid delay(); use millis()-based timing.
- Business logic in C++ only; the web UI is presentation.

Build/OTA: see .github/copilot-instructions.md
