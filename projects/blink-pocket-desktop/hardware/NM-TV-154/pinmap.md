# NM-TV-154 — Hardware ID (confirmed 2026-09-18)

Board detected on COM10 (CH340 USB-serial, VID_1A86&PID_7523).

## Source of truth
Nothing here is guessed. Confirmed two ways:
1. Direct device queries (esptool, boot log, live HTTP API on the running firmware).
2. Cross-checked against the manufacturer's own published config for this exact
   board: https://www.nmminer.com/2026/03/02/how-to-develop-nm-tv-custom-firmware/
   and https://github.com/RockBase-iot/NM-TV-154 (independent confirming source).
   Firmware itself: https://github.com/NMminer1024/NMMiner

## Chip
- Model: ESP32-D0WD-V3, revision v3.1 (classic dual-core ESP32, not S3)
- Cores: dual core + LP core, 240MHz
- Crystal: 40MHz
- MAC: 70:4b:ca:25:84:24
- Flash: 4MB (manufacturer ID 0x68, device 0x4016), DIO mode
- PSRAM: not reported by this board's firmware (no PSRAM line in /api/system/info)
- Radios: Wi-Fi + Bluetooth (classic ESP32 radio; BT profile support NOT yet verified)

## Display
- Controller: ST7789 (SPI)
- Resolution: 240x240 (confirmed live via device's own `/probe` endpoint: sw=240, sh=240)
- Driver library already in the shipped firmware: TFT_eSPI
- Pins (from manufacturer's own demo config, matches TFT_eSPI SPI pin scheme):
  - TFT_MOSI = GPIO13
  - TFT_SCLK = GPIO14
  - TFT_CS   = GPIO15
  - TFT_DC   = GPIO2
  - TFT_RST  = -1 (not wired; tied to chip reset)
  - TFT_MISO = -1 (not wired)
  - TFT_BL   = GPIO19 (PWM backlight, ACTIVE LOW)
  - SPI_FREQUENCY = 40000000
  - SPI_READ_FREQUENCY = 20000000

## Power
- GPIO21 controls the LCD power rail (per manufacturer README)

## Input
- One capacitive touch key on ESP32 touch channel T9 (= GPIO32)
- Tap / long-press only — this is NOT a touchscreen digitizer, no XPT2046/FT-series chip present
- No physical buttons confirmed beyond this touch pad (needs visual/photo confirmation if more exist)

## Storage
- No microSD slot on this board (not mentioned anywhere in firmware strings or manufacturer docs)
- Internal filesystem: LittleFS, ~384KB usable (fsTotal=393216 bytes reported live)

## Audio
- No microphone or speaker hardware on this board (nothing in firmware strings, not
  in manufacturer feature list). Audio (Phase 2 in BUILD_PLAN.md) will need an
  external I2S module if pursued on this exact board.

## NFC
- Not present. Do not build NFC features against this board without adding external hardware.

## Current factory firmware (as received)
- Product: NMMiner (open source, NMminer1024/NMMiner on GitHub)
- Firmware version: v2.0.01
- Hostname pattern: NMTV154_e6041c
- Actively solo-mining to solobtc.nmminer.com:3333 at boot, wallet
  bc1q4yykef7q4vsn5agg498qdflrrpr46n7wa05j9h (this is the factory-configured pool/wallet,
  not necessarily Zee's own — confirm before reusing in the miner app)
- Serves a live JSON API + web dashboard at its DHCP IP (was 192.168.0.136 at time
  of ID; will change), notably:
  - GET /api/system/info — identity, miner stats, storage
  - GET /probe — lightweight status incl. screen sw/sh
  - Full dashboard HTML at /

## Firmware backup
Full 4MB flash dump taken before any modification:
`C:\Temp\blink_backup\firmware_backup_full4mb.bin` (exact size 4,194,304 bytes,
matches confirmed flash size). Not yet copied into this repo (kept out of git —
see note below). Copy it somewhere durable before any flashing.

## Recovery / DFU
- USB CH340 serial, esptool.py talks to it normally in ROM bootloader mode
- Standard esptool auto-reset via RTS/DTR worked without needing manual boot-button entry
- No manual BOOT/RESET button sequence has been confirmed needed or present —
  needs visual confirmation if a full re-flash requires holding a button

## Not yet confirmed (do not assume)
- Whether Bluetooth is usable given this exact firmware's flash layout/partition table
- Any physical buttons beyond the single T9 touch pad
- Whether GPIO21 power-rail control conflicts with any other subsystem when repurposed
- Free GPIO available for new peripherals (needs partition/pin audit against this list)
