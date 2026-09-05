# Cardputer ADV Phone Mirror

Repository-ready prototype for mirroring an iPhone screen onto the M5Stack Cardputer ADV's 240x135 display.

**Made by Zeus / 925 Studios**

## What this is

This does **not** try to make an ESP32-S3 into a native AirPlay receiver. Instead it uses Apple's ReplayKit Broadcast Upload Extension to capture the iPhone screen, resize it to the Cardputer's native 240x135 resolution, JPEG-compress each frame, and stream those frames over local Wi-Fi to custom Cardputer firmware.

That architecture is intentionally chosen because the Cardputer ADV has an ESP32-S3FN8 at 240 MHz and an ST7789V2 240x135 display; full native AirPlay/H.264 receiver compatibility would be much heavier than the hardware needs for this use case.

## Repository layout

- `firmware/` — PlatformIO firmware for Cardputer ADV
- `ios/` — iPhone app + ReplayKit Broadcast Upload Extension, generated with XcodeGen
- `tools/pc_sender.py` — desktop sender used to prove the Cardputer video receiver before involving iOS
- `docs/PROTOCOL.md` — tiny TCP/JPEG framing protocol
- `FLASH_CARDPUTER_WINDOWS.bat` — one-click Windows build + flash launcher
- `windows/` — PowerShell flasher and troubleshooting instructions

## Fastest Windows flashing method

On Windows, you do not need to open VS Code or PlatformIO manually.

1. Download/clone this project folder to the Windows PC.
2. Set the Cardputer ADV side power switch to **OFF**.
3. Hold **G0**.
4. While holding G0, connect the Cardputer ADV with a **USB-C data cable**.
5. Release G0 after the USB cable is connected.
6. Double-click `FLASH_CARDPUTER_WINDOWS.bat`.
7. Follow the large on-screen instructions.

The flasher finds Python, installs PlatformIO if needed, builds the current firmware source, shows serial devices, and uploads the firmware. If multiple serial devices are attached, you can specify a port such as:

```bat
FLASH_CARDPUTER_WINDOWS.bat -Port COM5
```

See `windows/README.md` for troubleshooting.

## Milestone 1A — prove the Cardputer receiver

### One-click Windows route

Use `FLASH_CARDPUTER_WINDOWS.bat` as described above.

### Manual PlatformIO route

1. Install PlatformIO in VS Code.
2. Open the `firmware` folder.
3. Connect the Cardputer ADV over USB-C in download mode.
4. Build and upload.
5. Cardputer should show:
   - Wi-Fi: `CardputerMirror`
   - Password: `cardputer`
   - Host: `192.168.4.1:9000`
6. On a computer, connect Wi-Fi to `CardputerMirror`.
7. In `tools/`:

```bash
pip install -r requirements.txt
python pc_sender.py
```

The computer desktop should appear on the Cardputer screen at about 8 FPS. This step isolates and proves the ESP32 + LCD decoding path.

## Milestone 1B — iPhone full-screen broadcast

The iOS project uses a ReplayKit Broadcast Upload Extension, which is the Apple-supported way for an app extension to receive full-screen broadcast frames while you leave the app and use other apps.

### Build the iOS project

You need a current macOS/Xcode environment capable of signing an iPhone app.

1. Install XcodeGen (`brew install xcodegen`).
2. In the `ios` folder run:

```bash
xcodegen generate
open CardputerMirror.xcodeproj
```

3. In Xcode, choose your Apple development team for both targets.
4. Install the app on the iPhone.
5. Connect iPhone Wi-Fi to `CardputerMirror` / password `cardputer`.
6. Open Cardputer Mirror and tap the system broadcast button.
7. Start `Cardputer Mirror Broadcast`.
8. Leave the app. The phone screen should continue streaming to the Cardputer.

## Expected performance

Initial target: **240x135, 8 FPS, JPEG quality 45%**. This is deliberately conservative. Once it is stable on the actual Cardputer ADV we can tune quality/FPS and move to a shared-LAN transport.

## Important limitations

- DRM/secure video may be blanked by iOS/ReplayKit.
- This first version creates its own Cardputer Wi-Fi network. The iPhone may not have normal LAN access while joined to it.
- That network arrangement is fine for proving phone mirroring, but **Steam Link phase 2 must use shared-LAN mode** so the iPhone can reach the gaming PC while also streaming frames to Cardputer.
- Code is prepared against documented M5Cardputer/M5GFX APIs but still needs hardware validation on the user's ADV before calling it production-ready.

## Phase 2 — Steam Link control (after mirroring works)

Do not run Steam Link natively on the ESP32-S3. Instead:

`Gaming PC -> Steam Link on iPhone -> ReplayKit -> Cardputer display`

Then add a return input path:

`Cardputer keyboard / buttons -> BLE HID -> iPhone (or PC)`

That reuses the working video pipeline and turns the Cardputer into a tiny terminal/controller for the real PC without pretending the ESP32 can run the Steam Link client itself.

## GitHub Copilot context pack

This repository includes repository-wide Copilot instructions, agent instructions, path-specific instructions, and a reusable bootstrap prompt. Start with `COPILOT_START_HERE.md`.
