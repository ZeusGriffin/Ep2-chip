# BLINK Pocket Desktop — Project Snapshot

Snapshot date: 2026-09-17
Backup branch: `backup/blink-pocket-desktop-2026-09-17`

## Chosen AI face direction
Use the Tech Talkies / Xiaozhi custom animated-face approach as the visual/behavior reference, adapted into an original BLINK/NODE face for this device.

Primary references:
- https://www.youtube.com/watch?v=aDaSp6zaqWM
- https://github.com/TechTalkies/Xiaozhi-for-XiaoESP32S3
- https://www.youtube.com/watch?v=0GeJkjKwjbI
- https://www.youtube.com/watch?v=PMwH1sBMu3I

Target face behavior:
- idle personality motion
- random blinking / eye movement
- listening expression
- thinking animation
- speaking expression
- audio-reactive mouth animation
- wake-word or push-to-talk path
- short answer text on screen
- AI face may also be the home screen/screensaver

## Product concept
A tiny modular BLINK/NODE pocket desktop built on the current ESP32 display/miner hardware.

Launcher apps:
- AI Face
- Desktop
- Wi-Fi Radar
- Miner
- Scribe
- Phone Hub
- Games
- Aquarium
- Home
- Audio
- Tools
- Settings

## Features

### AI / voice
- animated AI face
- listen / think / speak states
- optional wake word
- push-to-talk option
- short on-screen AI responses
- optional mic/speaker support when hardware allows
- Bluetooth audio deferred to Phase 2

### Desktop
- clock/date
- calendar
- checklist / to-do
- notes
- timer / stopwatch / countdown
- quick links
- weather via companion bridge
- PC CPU/GPU/RAM/temperature telemetry
- network status
- currently playing / media status
- notifications
- GitHub/build status
- printer status

### Wi-Fi Radar
- passive SSID scan
- RSSI / signal bars
- channel
- secured/open indicator
- strongest-first sorting
- animated radar view
- saved/known-network indicator
- current connection details
- manual/automatic refresh

### Bitcoin miner
- retain NerdMiner functionality where board-compatible
- mining status
- hash rate
- best share
- BTC/network information
- price / block information

### Scribe companion
- live waveform
- recording/listening state
- partial transcript
- AI state
- short response text
- timer
- AI-face sync
- optional low-resolution iPhone/Scribe mirror later

### Phone Hub
- phone battery / connection status
- reminders
- calendar summary
- contact shortcuts
- shared text and URLs
- quick image preview
- notification summaries
- phone/device commands
- activity log

### Games
- Snake
- Pong
- Breakout
- Tetris-style block game
- Street Spider mini-game / simple web-swing platformer
- runner
- maze / puzzle
- optional NES / GB / GBC support only if hardware/resources permit
- optional SD-card game library

### Aquarium
- animated fish/creatures
- bubbles
- plants/water motion
- optional feed interaction
- clock overlay
- screensaver mode
- selectable themes later

### Home / smart devices
- Home Assistant bridge
- Govee light status/control
- favorite scenes
- room status
- one-tap routines

### Audio — Phase 2
- Bluetooth audio output to tiny LEICEX speaker where supported
- optional internal I2S speaker/amp later
- AI speech output
- internet radio
- notification sounds
- audio visualizer
- local audio playback
- volume control

### Phone / PC remote
- media play/pause/next
- volume
- presentation next/back
- authorized HID shortcuts where supported
- PC remote-launch shortcuts
- Steam Link-style companion status rather than full Steam Link on ESP32

### Screen companion
- PC low-resolution frame streaming
- Scribe companion view
- selected phone-screen mirroring
- secondary status screen for Cardputer/PC
- external-display experiments only if resources allow

### BLE tools
- passive nearby BLE scan
- device name / RSSI
- service UUID summary
- connect to user-selected known services only
- phone/device bridge

### Optional NFC / CyberSync
Only if NFC hardware is present or intentionally added:
- own-tag workflows
- URL/text/contact shortcuts
- CyberSync trigger
- Phone Hub trigger
- Home-scene trigger

### Utility drawer
- UUID generator
- Base64 encode/decode
- hex/text converter
- SHA-256 calculator
- QR display
- IP / MAC / hostname
- battery / uptime / memory / storage diagnostics
- file browser
- SD-card diagnostics
- firmware/build information
- activity log

### UX / launcher
- clean BLINK/NODE desktop
- compact app grid
- fast boot
- physical Home/Back when available
- optional triple-tap/long-press launcher shortcut
- `Made by ZUZ` signature
- app-on-demand loading
- settings for brightness, volume, Wi-Fi, pairing, theme, timeout

## Build order

### Phase 1
1. identify exact chip/display/resources
2. back up current firmware where practical
3. minimal display test
4. BLINK launcher
5. AI Face
6. passive Wi-Fi Radar
7. system information
8. Miner if compatible
9. Aquarium
10. simple games
11. Settings / recovery
12. OTA only after USB recovery is proven

### Phase 1.5
- Scribe companion
- PC telemetry
- Phone Hub
- Home/Govee
- BLE tools
- SD/file tools

### Phase 2
- Bluetooth audio to LEICEX speaker
- optional internal audio
- AI voice output
- internet radio / notification sounds

### Phase 3
- low-resolution phone/Scribe mirroring
- PC secondary-screen streaming
- optional NFC/CyberSync
- expanded emulation if hardware permits
- external-display experiments if resources permit

## Safety / engineering rules
- no guessed pin maps
- preserve recovery path
- hide unsupported hardware-dependent apps instead of crashing
- passive Wi-Fi/BLE scanning only
- no deauth, cracking, credential capture, packet injection, or impersonation tools
- retain miner functionality where compatible
- keep audio independent from the first stable firmware
