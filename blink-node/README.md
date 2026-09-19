# BLINK NODE — ESP32 / EP2

Independent ESP32 target.

Dependencies:
- Arduino ESP32 core
- ArduinoJson

Build flags or a private header must define WIFI_SSID and WIFI_PASSWORD.
The device exposes the BLINK NODE Wire Protocol v1 over local HTTP on port 8787.
Apple Watch and Raspberry Pi remain separate projects.