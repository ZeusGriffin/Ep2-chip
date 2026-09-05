# Project status

## Goal 1 — iPhone screen on Cardputer ADV

Status: **prototype source prepared; Windows one-click flasher added; hardware validation pending**.

Prepared:
- Cardputer ADV Wi-Fi/TCP JPEG receiver
- 240x135 JPEG display path using M5GFX `drawJpg`
- PC desktop sender for receiver validation
- iOS ReplayKit Broadcast Upload Extension sender
- XcodeGen project definition
- Framed TCP protocol documentation
- One-click Windows launcher: `FLASH_CARDPUTER_WINDOWS.bat`
- PowerShell build/upload helper in `windows/flash_cardputer.ps1`
- Windows troubleshooting instructions in `windows/README.md`

Next validation:
1. Put the user's Cardputer ADV into ESP32-S3 download mode: power switch OFF, hold G0, connect USB-C data cable, release G0.
2. Double-click `FLASH_CARDPUTER_WINDOWS.bat` and verify the firmware builds and uploads.
3. Boot normally and verify the `CardputerMirror` screen/Wi-Fi appears.
4. Run `tools/pc_sender.py` and verify stable video.
5. Measure actual FPS/free heap.
6. Build/sign the ReplayKit app and verify iPhone broadcast.

## Goal 2 — Steam Link control

Status: **intentionally deferred until Goal 1 passes**.

Planned architecture: Steam Link remains on iPhone; Cardputer receives the iPhone broadcast and later provides keyboard/button input by BLE HID or a direct PC input bridge.
