# BLINK Pocket Desktop — Build Plan

Updated: 2026-09-17

## Goal
Build the first stable firmware around the existing ESP32 display/miner hardware, with modular apps and an easy recovery/update path.

## Phase 1 — First working firmware
1. Identify exact ESP32/ESP32-S3 board, display controller, flash, PSRAM, buttons/touch, storage, and USB recovery method.
2. Back up current firmware if practical.
3. Minimal display test.
4. BLINK launcher.
5. AI Face demo: idle, blink, listen, think, speak animation states.
6. Passive Wi-Fi Radar.
7. System Info app.
8. Miner mode if the existing board is compatible.
9. Aquarium screensaver.
10. Simple games: Snake, Pong, Breakout, block game, Street Spider mini-game.
11. Settings + safe reset/recovery.
12. OTA update only after USB recovery is proven.

## Phase 1.5 — Companion features
- Scribe companion view: waveform, transcript, timer, AI state, short response.
- Desktop telemetry from PC: CPU/GPU/RAM/temp, network, media, GitHub/build, printer status.
- Phone Hub: battery, reminders, calendar summary, shared text/URLs, notification summaries.
- Home/Govee bridge.
- BLE scanner and device status.
- SD-card file browser and diagnostics if supported.

## Phase 2 — Audio
Deferred until the core firmware is stable.

- Bluetooth audio output to the user's tiny LEICEX speaker where supported by the exact ESP32 and firmware stack.
- Optional internal I2S speaker/amp path later.
- AI speech output.
- Internet radio and notification sounds.

Do not block Phase 1 on audio.

## Phase 3 — Advanced extras
- Low-resolution Scribe/iPhone mirroring.
- PC secondary-screen streaming.
- Optional NFC/CyberSync through added NFC hardware only.
- More emulator/game support if flash/PSRAM/storage/input performance permits.
- External display experiments only if spare GPIO/bandwidth allow it.

## Design rules
- Modular apps.
- Hide unsupported hardware-dependent apps instead of crashing.
- No guessed pin maps.
- Keep the first firmware easy to recover.
- Fast boot, simple launcher, minimal blocking animations.
- Preserve miner functionality where compatible.
- Keep Bluetooth speaker work separate until Phase 2.
