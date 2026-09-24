# MASTER BUILD PROMPT — ESP32 / EP2

Build BLINK NODE ESP32 firmware independently.

LOCKS
- No watchOS or Raspberry Pi runtime code.
- Arduino/ESP-IDF-compatible architecture.
- Keep Wi-Fi credentials and API keys out of Git.
- Implement BLINK NODE Wire Protocol v1.
- Preserve existing EP2 hardware mappings until verified.
- Add display/audio/face modules behind hardware abstraction; do not guess pins.
- Boot sequence: NODE, small BY ZEUS, launcher.
- Verify real-board compile before declaring firmware final.