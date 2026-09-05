# Cardputer ADV Phone Mirror — Copilot Repository Instructions

You are working on **Cardputer ADV Phone Mirror**, a two-part embedded/iOS project created for ZeusGriffin.

## Mission

Phase 1 is the only current priority: make an iPhone screen appear on an M5Stack Cardputer ADV reliably.

Do **not** try to implement native AirPlay on the ESP32-S3. The intended architecture is:

`iPhone ReplayKit Broadcast Upload Extension -> JPEG frames over local TCP/Wi-Fi -> Cardputer ADV ESP32-S3 -> 240x135 ST7789V2 display`

Phase 2 is intentionally deferred until Phase 1 is hardware-validated:

`Gaming PC -> Steam Link on iPhone -> ReplayKit -> Cardputer display`

with a later return-input path from the Cardputer keyboard/buttons using BLE HID or a direct network input bridge.

## Current known hardware / constraints

- Target device: **M5Stack Cardputer ADV**.
- MCU family: ESP32-S3.
- Display target: 240x135 ST7789V2-class panel through M5Cardputer/M5GFX APIs.
- Cardputer-side firmware framework: Arduino + PlatformIO.
- iPhone side: Swift + ReplayKit Broadcast Upload Extension + Network.framework.
- Initial transport: TCP port 9000.
- Initial receiver Wi-Fi AP:
  - SSID: `CardputerMirror`
  - password: `cardputer`
  - address: `192.168.4.1:9000`
- Initial frame format: baseline JPEG, target 240x135, ~8 FPS, quality ~0.45.
- Receiver frame-size limit: 120 KiB.
- Wire protocol is documented in `docs/PROTOCOL.md`.

## Repository map

- `firmware/` — Cardputer ADV receiver firmware.
- `ios/` — iPhone app + ReplayKit Broadcast Upload Extension.
- `tools/pc_sender.py` — desktop sender used to validate the Cardputer receiver before iPhone testing.
- `docs/` — architecture, protocol, test plan, troubleshooting, and phase-2 notes.
- `PROJECT_STATUS.md` — authoritative progress ledger.

## Engineering rules

1. **Do not claim hardware success unless a real-device test has passed.** Distinguish source-complete, compile-tested, and hardware-tested states.
2. **Keep Phase 1 simple and testable.** Prove PC sender -> Cardputer before iPhone ReplayKit.
3. **Do not replace the architecture with native AirPlay/H.264 on ESP32-S3** unless there is hard evidence it is practical on this hardware and the user explicitly asks for that experiment.
4. Favor small, reversible changes. Preserve a working receiver while adding features.
5. Do not silently change the protocol. If protocol bytes change, update `docs/PROTOCOL.md` and bump the protocol version.
6. Avoid dynamic allocation churn inside the frame loop when possible. Watch free heap and JPEG frame size.
7. Keep display work at native 240x135 unless testing proves another path is better.
8. Preserve serial diagnostics for FPS, free heap, sender connection/disconnection, and errors.
9. For iOS, keep the broadcast extension memory-conscious. Drop frames rather than queueing unbounded work.
10. DRM/secure video may be black/blank under ReplayKit; document this rather than treating it as a receiver bug.
11. Do not add Steam Link control work until the Phase 1 acceptance checks pass.
12. Do not commit generated build output, Xcode DerivedData, `.pio`, secrets, signing credentials, or provisioning profiles.

## Phase 1 acceptance checks

Phase 1 is complete only when all of these are true on real hardware:

- Firmware builds and flashes to the Cardputer ADV.
- Cardputer boots and creates the expected Wi-Fi AP.
- `tools/pc_sender.py` can connect and display a moving desktop image.
- Receiver runs for at least 10 minutes without crash/reboot.
- Actual FPS and free-heap behavior are recorded.
- iPhone app builds and signs.
- ReplayKit broadcast extension connects to the Cardputer.
- iPhone home screen / ordinary apps visibly update on the Cardputer.
- Orientation behavior is acceptable in both portrait and landscape testing.
- Reconnect after sender disconnect works without reflashing.

## What to do when asked to continue the project

1. Read `PROJECT_STATUS.md` first.
2. Read `docs/TEST_PLAN.md` and `docs/ARCHITECTURE.md`.
3. Inspect the relevant source files before editing.
4. State what is already proven versus still assumed.
5. Make the smallest change that advances the next unpassed acceptance check.
6. Update `PROJECT_STATUS.md` after any meaningful implementation or test result.

## Style / ownership

- Project credit: **Made by Zeus / 925 Studios**.
- Prefer concise, practical comments that explain hardware/network intent.
- Do not remove working diagnostics just to make code look cleaner.
