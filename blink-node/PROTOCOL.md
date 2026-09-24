# BLINK NODE Wire Protocol v1

POST /api/v1/command
{"id":"id","source":"watch|pi|esp32","action":"ping|wake|sleep|listen|stop|status|led|launch","value":"optional","timestamp":0}

GET /api/v1/status
{"ok":true,"device":"BLINK NODE","platform":"esp32","state":"idle","battery":-1,"version":"0.1.0"}

Keep Apple Watch, Raspberry Pi, and ESP32 build systems independent.