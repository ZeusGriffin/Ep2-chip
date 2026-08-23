# Wuzplay Cyberdeck v8 capability boundary

## Verified compiled-firmware markers

The gold `pixljs.bin` contains custom markers for the Cyberdeck application/menu, Cyber Dashboard, Cyber Tools, Video Player, BLE File Transfer, Card Emulator, Amiibo Database, and the game set including Arkanoid, Invaders, Lander, Tris, and Cyber Shadow. The release also preserves the four-button layout (Left, Select, Right, Back) established by the package instructions.

This document intentionally does not claim that phone-assisted features are compiled firmware.

## NFC / phone companion capabilities

The v8 companion layer contains 19 NFC actions:

- CyberSync System
- CyberSync Dashboard
- CyberSync Network
- CyberSync Tools
- CyberSync Alerts
- CyberSync All
- Open WuzSync
- Drive Home
- Find Car
- Flashlight
- Quick Note
- 10-Minute Timer
- Govee On
- Govee Off
- Govee Bright
- Govee Relax
- Govee Movie
- Govee Red
- Govee Blue

Govee actions use the architecture `Wuzplay NFC emulation -> iPhone -> Apple Shortcut -> Govee action`. No Govee API key or account token belongs in the DFU.

## What remains to make source reproducible

The verified DFU is a binary gold reference. A future source port must reproduce its behavior from source, build a fresh Nordic DFU, then pass device testing before replacing the gold binary. Until then, the verified binary is preserved unchanged.
