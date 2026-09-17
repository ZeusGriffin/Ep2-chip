# BLINK Pocket Desktop / AI Face

Status: hardware identification required before firmware flash
Updated: 2026-09-17

## Goal
Turn the existing ESP32 display/miner into one small multi-mode device rather than replacing one firmware with another.

Boot -> BLINK launcher -> AI Face / Desktop / Miner / Scribe / Games / Aquarium / Home / Audio / Settings

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
- I2S microphone input
- I2S amplifier + speaker output when hardware permits
- OTA/DFU update path
- configurable assistant name / voice / language / role
- non-blocking face animation so networking/audio stay responsive

Important: do not flash the reference binary directly until the actual board, display controller, flash size, PSRAM, GPIO map, microphone, speaker/amp, and USB/DFU method are confirmed.

## Scribe mode
Two implementation levels:

### Preferred: Scribe companion view
Scribe sends only what the little screen needs over Wi-Fi/BLE:
- live waveform / listening indicator
- partial transcript
- AI thinking state
- short response text
- animated AI face state
- timer / recording status

This will be faster and smoother than sending the whole iPhone display.

### Optional: phone screen mirror
A companion iPhone app can capture selected screen content with Apple's screen-capture APIs, resize/compress frames, and stream low-resolution frames over Wi-Fi to the ESP32. Expect lower frame rate and more latency than the native Scribe companion view.

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
Reference implementations:
- https://github.com/Lagerpun/esp32-cyd-aquarium
- https://github.com/POWER-PILL/ASCII-Aquarium

Target:
- autonomous animated fish/creatures
- bubbles/plants/water motion
- optional touch-to-feed
- clock overlay
- screensaver behavior after idle timeout

## Games mode
Build as a launcher with board-appropriate games only after hardware identification.
Potential categories:
- Snake
- Pong
- Breakout
- Tetris-style block game
- simple platformer
- simple spider/web-swing inspired mini-game
- lightweight retro emulation only if flash/PSRAM/input hardware supports it

Do not promise GBA/N64-class emulation on an unknown ESP32 board.

## Home / Audio
- Home Assistant / Govee status and commands through a network bridge
- internet radio when audio hardware exists
- notification sounds
- audio visualizer

## Firmware architecture
1. Hardware abstraction layer
   - display
   - touch/buttons
   - audio in/out
   - Wi-Fi/BLE
   - storage
2. App manager / launcher
3. AI face state machine
4. Scribe companion protocol
5. Desktop telemetry client
6. Miner app
7. Aquarium app
8. Game launcher
9. Home/audio modules
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
8. add AI face
9. add audio only after GPIO/voltage verification
10. add Scribe/Desktop/Aquarium/Games incrementally

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
