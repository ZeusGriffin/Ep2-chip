# ESP32 / NerdMiner / AI Video Research

Updated: 2026-09-17

Purpose: quick reference for turning a small ESP32 Bitcoin/NerdMiner display into a more useful multi-mode device while preserving the miner function.

## 1) NerdMiner / Bitcoin miner

### How to Create a NerdMiner with ESP32 — Step-by-Step Tutorial for Mining Bitcoin
YouTube: https://www.youtube.com/watch?v=wBotsMK20Qc

Good for: flashing NerdMiner firmware, Wi-Fi setup, wallet configuration, pool setup, and viewing mining statistics.

### ESP32 Mining Setup Guide — NerdMiner V2 WiFi, Multi-Pool & Config Page Walkthrough
YouTube: https://www.youtube.com/watch?v=xob4d2rilMk

Good for: NerdMiner V2 setup, configuration page, multiple pools, rebooting/changing pools, and avoiding common setup problems.

Core project:
https://github.com/BitMaker-hub/NerdMiner_v2

## 2) AI voice assistant

### Build Your Own AI Voice Assistant with ESP32 + ChatGPT + ElevenLabs + INMP441
YouTube: https://www.youtube.com/watch?v=iBCNKnP-h70

Good for: adding speech input/output. Uses an INMP441 I2S microphone, Whisper speech-to-text, ChatGPT, ElevenLabs TTS, MAX98357A I2S audio amp, and a speaker.

Potential BLINK/NODE use: press-to-talk AI mode while the ESP32 acts as the display/audio endpoint and a cloud/PC service handles the heavy AI work.

## 3) Smart-home / desk dashboard

### Customising an ESP32-S3 Smart Display with Home Assistant
YouTube: https://www.youtube.com/watch?v=Km-fjUZfnd0

Good for: multi-page ESP32 dashboards, buttons, clock, weather, smart-home status, and Home Assistant controls.

Potential BLINK/NODE use: HOME mode for lights, scenes, weather, room status, and quick controls.

## 4) PC hardware monitor

### ESP32 PC State Display — CPU & GPU Temperature & Usage
YouTube: https://www.youtube.com/watch?v=rH2XUyTgyKc

Good for: using an ESP32 display as a tiny PC telemetry screen showing CPU/GPU usage and temperatures.

Potential BLINK/NODE use: DESK mode for CPU, GPU, RAM, network, GitHub/build status, printer status, or notifications.

## 5) Audio / speaker / microphone / internet radio

### ESP32 Sound — Working with I2S
YouTube: https://www.youtube.com/watch?v=m-MPBjScNRk

Good for: I2S microphone input, MP3 playback, I2S amplifier output, internet radio, stereo audio, and microphone waveforms.

Potential BLINK/NODE use: speaker + microphone foundation for AI voice, notification sounds, internet radio, or audio visualization.

## Recommended device modes

- MINER — retain NerdMiner mining and Bitcoin stats
- AI — press-to-talk AI assistant with screen + optional speaker
- DESK — PC status, notifications, GitHub/build status
- HOME — Home Assistant / Govee / smart-home controls
- AUDIO — internet radio, MP3, alerts, waveform visualizer

## Hardware ideas to research after exact board identification

- INMP441 I2S microphone
- MAX98357A I2S amplifier
- 8 ohm mini speaker
- spare GPIO availability
- internal case space
- button mapping
- USB power budget
- whether the existing board is ESP32, ESP32-S3, or another supported NerdMiner target

## Next step

Identify the exact board inside the miner before changing firmware or wiring. Once confirmed, make a board-specific pin map and determine whether voice/audio can be added without replacing the main board.
