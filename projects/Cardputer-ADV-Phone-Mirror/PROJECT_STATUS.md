# Project status

## Goal 1 — iPhone screen on Cardputer ADV

Status: **prototype source prepared; hardware validation pending**.

Prepared:
- Cardputer ADV Wi-Fi/TCP JPEG receiver
- 240x135 JPEG display path using M5GFX `drawJpg`
- PC desktop sender for receiver validation
- iOS ReplayKit Broadcast Upload Extension sender
- XcodeGen project definition
- Framed TCP protocol documentation

Next validation:
1. Compile/flash firmware on the user's Cardputer ADV.
2. Run `tools/pc_sender.py` and verify stable video.
3. Measure actual FPS/free heap.
4. Build/sign the ReplayKit app and verify iPhone broadcast.

## Goal 2 — Steam Link control

Status: **intentionally deferred until Goal 1 passes**.

Planned architecture: Steam Link remains on iPhone; Cardputer receives the iPhone broadcast and later provides keyboard/button input by BLE HID or a direct PC input bridge.
