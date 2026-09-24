# BLINK Pocket Desktop / AI Face

Status: hardware identified (NM-TV-154, ESP32-D0WD-V3, ST7789 240x240) — see hardware/NM-TV-154/pinmap.md. Firmware backed up. Display, gestures, wired/BLE input, WiFi OTA, page nav, and vector-font styling are BUILT + COMPILE-VERIFIED but NOT YET flashed to the physical board (device wasn't connected to do so). See archive/2026-09-24-handoff.md for the exact state and the blocked/unverified list.
Updated: 2026-09-24

## Goal
Turn the existing ESP32 display/miner into one small modular multi-mode device rather than replacing one firmware with another.

Boot -> BLINK launcher -> AI Face / Desktop / Wi-Fi Radar / Miner / Scribe / Phone Hub / Games / Aquarium / Home / Audio / Tools / Settings

The firmware should use a modular app manager so features can be added or removed later without rebuilding the whole concept.

## Core apps

### AI Face
Reference builds:
- Tech Talkies: I Built an AI Desk Buddy with ESP32 (Xiaozhi + Custom Face UI)
  - https://www.youtube.com/watch?v=aDaSp6zaqWM
  - https://github.com/TechTalkies/Xiaozhi-for-XiaoESP32S3
- Tech Talkies Part 2: GC9A01 TFT / newer animated face
  - https://www.youtube.com/watch?v=0GeJkjKwjbI
- Generic ESP32-S3 face integration walkthrough
  - https://www.youtube.com/watch?v=PMwH1sBMu3I

Target behavior:
- idle personality motion
- random blinking / eye tracking
- listening expression
- thinking state
- speaking expression
- audio-reactive mouth animation
- wake word or push-to-talk
- configurable assistant name / voice / language / role
- short on-screen answers
- optional I2S microphone and speaker when hardware permits
- non-blocking animation so networking/audio stay responsive

### Desktop
Pull forward the useful Cardputer Cyber Arcade ideas:
- clock/date
- calendar
- to-do list
- timer / stopwatch / countdown
- notes
- quick links
- weather through phone/PC bridge
- PC CPU/GPU/RAM/temperature telemetry
- network status
- media / currently playing
- notifications through phone/PC bridge
- GitHub/build status
- printer status
- quick-launch tiles

Reference project: ZeusGriffin/CardputerCyberArcade

### Wi-Fi Radar
Passive nearby-network viewer only:
- scan SSIDs
- RSSI / signal bars
- channel
- secured/open indicator
- strongest-first sorting
- simple animated radar view
- saved known-network indicator
- current connection details
- refresh button / auto refresh interval

No deauth, cracking, credential capture, packet injection, or impersonation features.

### Miner
Keep existing Bitcoin/NerdMiner functionality where compatible:
- mining status
- hash rate
- best share
- BTC/network information
- price / block information

Reference:
https://github.com/BitMaker-hub/NerdMiner_v2

### Scribe
Preferred companion mode:
- live waveform
- listening/recording state
- partial transcript
- AI thinking state
- short response text
- animated AI face state
- timer / recording status

Optional mirror mode:
- low-resolution selected iPhone screen stream over Wi-Fi
- prioritize Scribe companion data over full video for responsiveness

### Phone Hub
Bring forward the earlier CyberSync/WuzSync concept:
- phone battery / connection status
- reminders
- calendar summary
- contacts shortcuts
- shared text / URLs
- quick photo-to-small-screen preview
- notification summaries through companion app
- send simple commands between phone and device
- Phone Hub activity log

### Games
Launcher with hardware-appropriate games:
- Snake
- Pong
- Breakout
- Tetris-style block game
- Street Spider / simple web-swing platform game
- simple runner
- maze / puzzle
- lightweight NES / Game Boy / Game Boy Color support only if the exact chip, flash, PSRAM, storage, controls, and performance support it
- SD-card game library when microSD is available

Do not promise GBA/N64-class emulation on an unknown ESP32 board.

### Aquarium
References:
- https://github.com/Lagerpun/esp32-cyd-aquarium
- https://github.com/POWER-PILL/ASCII-Aquarium

Target:
- autonomous animated fish/creatures
- bubbles/plants/water motion
- optional touch/button feeding
- clock overlay
- screensaver after idle timeout
- selectable aquarium themes later

### Home
- Home Assistant bridge
- Govee light status/control
- favorite scenes
- room status
- one-tap routines
- device states supplied through phone/PC/home bridge

### Audio
When audio hardware exists:
- internet radio
- notification sounds
- audio visualizer
- simple local audio player from storage
- AI speech output
- volume control

## Extra modules pulled from previous projects

### Phone / PC remote
- simple media play/pause/next controls
- volume controls
- presentation next/back
- mouse/keyboard style commands only where the board supports HID and the paired computer authorizes it
- PC status / remote launcher shortcuts
- Steam Link-style companion status rather than trying to run full Steam Link on the ESP32

### Screen companion
- PC -> device low-resolution frame streaming
- Scribe -> device companion view
- selected phone-screen mirroring
- secondary status screen for the Cardputer or PC
- optional external-display output later if the board has enough spare GPIO/bandwidth

### BLE tools
- scan nearby BLE advertisements
- show device name, RSSI, service UUID summary
- connect only to user-selected devices that support a known service
- phone/device bridge

Passive/authorized use only.

### NFC / CyberSync expansion
Only if this exact hardware includes NFC or an external PN532-class module is intentionally added:
- own-tag reader/emulator workflows
- saved personal NFC actions
- URL/text/contact shortcuts
- CyberSync dashboard trigger
- Phone Hub trigger
- Home scene trigger

Do not pretend the base ESP32 has NFC when it does not.

### Utility drawer
Small offline tools from earlier WuzDeck concepts:
- UUID generator
- Base64 encode/decode
- hex/text converter
- SHA-256 calculator
- QR display for text/URL when screen resolution allows
- IP / MAC / hostname display
- battery / uptime / memory / storage diagnostics
- file browser
- SD-card diagnostics
- firmware/build information
- activity log

### Personal organizer
- checklist
- timer
- stopwatch
- countdown
- notes
- calendar snapshot
- reminders from phone bridge
- favorite links

### Media / web shortcuts
- phone-assisted YouTube search/results
- open URL on paired phone/computer
- currently-playing track
- basic media controls
- saved stations / links

The ESP32 should not attempt to be a full modern web browser.

## Launcher / UX
- clean BLINK/NODE desktop
- compact app grid
- AI face can be the home screen/screensaver
- physical Home / Back behavior where buttons exist
- triple-tap or long-press shortcut to launcher if hardware supports it
- tiny `Made by ZUZ` signature
- fast boot; no long blocking intro
- apps load on demand
- remember last app when safe
- settings page for brightness, volume, Wi-Fi, companion pairing, theme, timeout

## Storage / app architecture
Use capability detection instead of assuming every feature exists.

1. Hardware abstraction layer
   - display
   - touch/buttons
   - audio in/out
   - Wi-Fi/BLE
   - storage / microSD
   - optional NFC
2. App manager / launcher
3. Shared event bus
4. AI face state machine
5. Phone/Scribe companion protocol
6. Desktop telemetry client
7. Passive Wi-Fi/BLE scanners
8. Miner app
9. Aquarium app
10. Game launcher
11. Home/audio modules
12. Utility drawer
13. Settings + recovery

Apps that require unavailable hardware should hide themselves or show `hardware not installed` rather than fail.

## AI state machine
IDLE -> WAKE/LISTEN -> THINK -> SPEAK -> IDLE

Events:
- wake word or button
- VAD/audio start
- transcript ready
- AI response streaming
- TTS/audio level drives mouth animation
- timeout returns to idle face/screensaver

## Update / recovery design
Make updates easy after the first board-specific build:
- USB DFU / serial recovery
- OTA update when supported
- version/build info on-device
- safe settings reset
- keep a minimal recovery build
- optional SD-based assets/apps when storage exists
- never overwrite board-specific pin maps with generic guesses

## DFU / safety plan
Before writing firmware:
1. identify exact board/chip from PCB or USB descriptor
2. identify display controller and resolution
3. confirm flash + PSRAM
4. save/read current firmware if practical
5. document boot/DFU buttons and recovery sequence
6. compile a minimal display test
7. test launcher
8. add Wi-Fi/BLE capability detection
9. add AI face
10. add audio only after GPIO/voltage verification
11. add Scribe/Desktop/Miner/Aquarium/Games incrementally
12. enable OTA only after USB recovery is proven

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
- storage/microSD support
- existing mic/speaker hardware, if any
- any NFC/BLE-specific peripherals

Once these are known, create `hardware/<exact-board>/pinmap.md` and a board-specific build target before DFU.

## First firmware target
Keep the first flash intentionally simple and stable:
1. BLINK launcher
2. AI Face demo
3. passive Wi-Fi Radar
4. device/system info
5. Miner if existing board support is confirmed
6. Aquarium screensaver
7. simple games
8. Settings / recovery

Then add Scribe, Phone Hub, audio, PC telemetry, Home/Govee, BLE, mirroring, and optional peripherals as modules.
