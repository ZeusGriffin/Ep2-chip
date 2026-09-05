---
applyTo: "firmware/**"
---
# Firmware-specific instructions

Target the M5Stack Cardputer ADV / ESP32-S3 using PlatformIO and the M5Cardputer library.

- Preserve `M5Cardputer.begin(...)`, display initialization, and serial diagnostics unless a change is required by a verified hardware issue.
- Keep initial display target at 240x135 and use M5GFX/M5Cardputer display APIs.
- Default TCP server port is 9000.
- Validate every received frame header before allocation/decode.
- Keep a hard maximum frame size.
- Favor buffer reuse instead of per-frame allocation.
- Avoid blocking loops that prevent recovery/reconnect indefinitely.
- Keep an obvious on-device waiting/error status screen.
- Report FPS and free heap at runtime.
- When adding shared-LAN mode later, preserve AP mode as a recovery/test path.
