# BLINK Pocket Desktop / AI Face

Status: hardware identification required before firmware flash
Updated: 2026-09-17

## Goal
Turn the existing ESP32 display/miner into one simple multi-mode device.

Boot -> BLINK launcher -> AI Face / Desktop / Wi-Fi Radar / Miner / Scribe / Games / Aquarium / Settings

## Keep V1 easy
First firmware target should only include the pieces needed to prove the device works:

1. BLINK launcher
2. AI Face
3. Wi-Fi Radar
4. Desktop/status page
5. Scribe companion page
6. Aquarium screensaver
7. Games launcher with simple board-safe games
8. Miner page if compatible
9. Settings + OTA/DFU recovery

Home controls, audio/radio, advanced phone mirroring, and heavier emulation can come later.

## AI Face target
Reference builds:
- Tech Talkies: I Built an AI Desk Buddy with ESP32 (Xiaozhi + Custom Face UI)
  - https://www.youtube.com/watch?v=aDaSp6zaqWM
  - https://github.com/TechTalkies/Xiaozhi-for-XiaoESP32S3
- Tech Talkies Part 2: GC9A01 TFT / newer animated face
  - https://www.youtube.com/watch?v=0GeJkjKwjbI
- Generic ESP32-S3 face integration walkthrough
  - https://www.youtube.com/watch?v=PMwH1sBMu3I

Features to preserve/adapt:
- idle personality motion
- random blinking
- listening expression
- thinking state
- speaking expression
- audio-reactive mouth animation
- wake word / push-to-talk option
- I2S microphone input when hardware permits
- I2S amplifier + speaker output when hardware permits
- OTA/DFU update path
- configurable assistant name / voice / language / role
- non-blocking face animation so networking/audio stay responsive

Important: do not flash the reference binary directly until the actual board, display controller, flash size, PSRAM, GPIO map, microphone, speaker/amp, and USB/DFU method are confirmed.

## Wi-Fi Radar
Simple visual network scanner using the ESP32 Wi-Fi radio.

V1 behavior:
- tap Wi-Fi Radar
- scan nearby Wi-Fi access points
- show SSID
- show signal strength (RSSI)
- show channel
- show open/secured status
- sort strongest first
- optional radar-style circles for signal strength
- rescan button

Keep this passive: scan and display only. No deauthentication, password attacks, interception, or packet injection.

## Scribe mode
### Preferred: Scribe companion view
Scribe sends only what the little screen needs over Wi-Fi/BLE:
- live waveform / listening indicator
- partial transcript
- AI thinking state
- short response text
- animated AI face state
- timer / recording status

This will be faster and smoother than sending the whole iPhone display.

### Optional later: phone screen mirror
A companion iPhone app can capture selected screen content, resize/compress frames, and stream low-resolution frames over Wi-Fi to the ESP32. Expect lower frame rate and more latency than the native Scribe companion view.

## Desktop mode
- clock/date
- PC CPU/GPU/RAM/temperature telemetry
- network/Wi-Fi status
- current music / media status
- notifications
- GitHub/build status
- printer status
- quick-launch tiles

## Miner mode
Keep the existing Bitcoin/NerdMiner functionality where compatible:
- mining status
- hash rate
- best share
- BTC/network information
- price / block information

Reference:
https://github.com/BitMaker-hub/NerdMiner_v2

## Aquarium mode
References:
- https://github.com/Lagerpun/esp32-cyd-aquarium
- https://github.com/POWER-PILL/ASCII-Aquarium

Target:
- autonomous animated fish/creatures
- bubbles/plants/water motion
- optional touch-to-feed
- clock overlay
- screensaver behavior after idle timeout

## Games mode
Board-safe first set:
- Snake
- Pong
- Breakout
- Tetris-style block game
- simple spider/web-swing inspired mini-game

Lightweight retro emulation only if flash/PSRAM/input hardware supports it.

## Firmware architecture
1. Hardware abstraction layer
   - display
   - touch/buttons
   - audio in/out
   - Wi-Fi/BLE
   - storage
2. App manager / launcher
3. AI face state machine
4. Wi-Fi Radar scanner
5. Scribe companion protocol
6. Desktop telemetry client
7. Miner app
8. Aquarium app
9. Game launcher
10. Settings + OTA/DFU recovery

## AI state machine
IDLE -> WAKE/LISTEN -> THINK -> SPEAK -> IDLE

Events:
- wake word or button
- VAD/audio start
- transcript ready
- AI response streaming
- TTS/audio level drives mouth animation
- timeout returns to idle face/screensaver

## DFU / safety plan
Before writing firmware:
1. identify exact board/chip from PCB or USB descriptor
2. identify display controller and resolution
3. confirm flash + PSRAM
4. save/read current firmware if practical
5. document boot/DFU buttons and recovery sequence
6. compile a minimal display test
7. test launcher
8. test Wi-Fi Radar
9. add AI face
10. add audio only after GPIO/voltage verification
11. add Scribe/Desktop/Aquarium/Games incrementally

Never wire or flash using a guessed pin map.

## First physical-session checklist
When the device is plugged into the computer, collect:
- USB VID/PID and serial device
- ESP chip model/revision
- flash size
- PSRAM presence/size
- current firmware/boot log
- display controller/resolution
- button/touch inputs
- available GPIO
- existing mic/speaker hardware, if any

Once these are known, create `hardware/<exact-board>/pinmap.md` and a board-specific build target before DFU.
