# One-Click Windows Flashing

Use the file at the project root:

`FLASH_CARDPUTER_WINDOWS.bat`

## What it does

1. Finds Python 3 (`py -3` or `python`).
2. Installs PlatformIO automatically if it is missing.
3. Pauses while you put the Cardputer ADV into download mode.
4. Shows detected serial devices.
5. Builds the firmware in `firmware/`.
6. Uploads it to the Cardputer ADV.
7. Prints the expected CardputerMirror Wi-Fi details after a successful flash.

## Cardputer ADV download mode

1. Set the side power switch to **OFF**.
2. Hold **G0**.
3. Connect the Cardputer ADV to the Windows PC with a **USB-C data cable** while continuing to hold G0.
4. Release G0 after the cable is connected.

This is the ESP32-S3 download/bootloader mode used to flash firmware.

## Normal use

Double-click:

`FLASH_CARDPUTER_WINDOWS.bat`

The script lets PlatformIO detect the COM port automatically.

If Windows has multiple ESP/serial devices attached and auto-detection chooses the wrong one, open Command Prompt in the project folder and run:

```bat
FLASH_CARDPUTER_WINDOWS.bat -Port COM5
```

Replace `COM5` with the Cardputer ADV port shown by the script.

## After flashing

1. Disconnect USB.
2. Move the side switch to **ON**.
3. Power the Cardputer ADV normally.
4. The screen should show `CARDPUTER MIRROR`.
5. The expected receiver network is:
   - Wi-Fi: `CardputerMirror`
   - Password: `cardputer`
   - Host: `192.168.4.1:9000`

## If flashing fails

- Confirm the cable supports USB data.
- Re-enter download mode: OFF -> hold G0 -> connect USB -> release G0.
- Close M5Burner, Arduino Serial Monitor, PlatformIO Serial Monitor, or other apps using the same COM port.
- Try specifying the COM port manually.
- If needed, restore official Cardputer ADV factory firmware with M5Burner, then retry the project firmware.

## Important

The flash package builds the current source before uploading it. This means GitHub Copilot or future code changes are included automatically the next time the batch file is run.
