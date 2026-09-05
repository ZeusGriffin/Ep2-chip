# Architecture

## Phase 1: iPhone -> Cardputer ADV

The project intentionally avoids implementing a native AirPlay receiver on the ESP32-S3.

### Data path

```text
iPhone screen
   |
   v
ReplayKit Broadcast Upload Extension
   |
   | crop + resize to 240x135
   | JPEG encode (~45% initial quality)
   v
Network.framework TCP sender
   |
   | CMIR frame protocol / port 9000
   v
Wi-Fi
   |
   v
Cardputer ADV ESP32-S3
   |
   | validate header + length
   | reuse frame buffer
   | M5GFX JPEG decode
   v
240x135 display
```

### Why this architecture

- The iPhone already has the compute needed to capture, resize, and JPEG-compress its screen.
- The Cardputer only needs to receive, decode, and draw small JPEG frames.
- JPEG-over-TCP is easy to debug with a PC sender before involving iOS signing/ReplayKit.
- The design reduces memory and codec complexity on the ESP32-S3.

## Test architecture

Before iPhone testing:

```text
Computer desktop -> tools/pc_sender.py -> Wi-Fi/TCP -> Cardputer ADV
```

This isolates the receiver, transport, and display path.

## Network modes

### Mode A — Cardputer access point (current)

Cardputer creates:

- SSID `CardputerMirror`
- password `cardputer`
- address `192.168.4.1`

Best for Phase 1 bring-up because it removes router/discovery variables.

### Mode B — shared LAN (planned after Phase 1)

Cardputer and iPhone join the same existing Wi-Fi network. This is required for the later Steam Link phase because the iPhone must reach the gaming PC while simultaneously sending frames to the Cardputer.

## Phase 2: Steam Link terminal

Planned, not implemented:

```text
Gaming PC -> Steam Link on iPhone -> ReplayKit -> Cardputer display
```

Possible input return path:

```text
Cardputer keyboard/buttons -> BLE HID or network bridge -> iPhone / gaming PC
```

The ESP32-S3 is not expected to run a full Steam Link client itself.
