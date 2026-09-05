# Phase 1 Test Plan

Record every test result in `PROJECT_STATUS.md`.

## Test 0 — source sanity

- [ ] PlatformIO project resolves dependencies.
- [ ] Firmware compiles for the configured ESP32-S3 target.
- [ ] Python sender starts and can capture/encode frames.
- [ ] XcodeGen project definition is syntactically valid.

## Test 1 — flash / boot

- [ ] Connect Cardputer ADV by a known data-capable USB cable.
- [ ] Build and upload firmware.
- [ ] Device reboots without boot loop.
- [ ] Display shows the Cardputer Mirror waiting screen.
- [ ] Serial monitor shows ready state and IP/port.

Capture:
- exact upload result
- serial boot log
- firmware build environment

## Test 2 — Wi-Fi AP

- [ ] `CardputerMirror` appears in Wi-Fi list.
- [ ] Password `cardputer` works.
- [ ] Client receives an address.
- [ ] `192.168.4.1` is reachable.

## Test 3 — PC sender

- [ ] Connect computer to `CardputerMirror`.
- [ ] Install `tools/requirements.txt`.
- [ ] Run `python pc_sender.py`.
- [ ] Cardputer display shows moving desktop content.
- [ ] No persistent JPEG decode errors.
- [ ] Sender reconnects after stopping/restarting script.

Measure:
- FPS
- average / worst visible latency
- free heap
- typical JPEG frame size

## Test 4 — stability

- [ ] Stream for 10 minutes.
- [ ] No reboot.
- [ ] No progressive memory loss.
- [ ] Display continues updating.
- [ ] Disconnect/reconnect works.

## Test 5 — iPhone build

- [ ] Generate Xcode project.
- [ ] Select valid Apple development team for app and extension.
- [ ] Build succeeds.
- [ ] Install succeeds on physical iPhone.
- [ ] Broadcast extension appears in the system broadcast picker.

## Test 6 — iPhone broadcast

- [ ] iPhone connected to `CardputerMirror` Wi-Fi.
- [ ] Start Cardputer Mirror Broadcast.
- [ ] App/home screen appears on Cardputer.
- [ ] Leave host app; broadcast continues.
- [ ] Test portrait orientation.
- [ ] Test landscape orientation.
- [ ] Stop/restart broadcast and verify reconnect.

## Phase 1 pass gate

Do not start Phase 2 until Tests 1-6 have passed on real hardware, with any known limitations documented.
