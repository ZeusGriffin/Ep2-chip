# Phase 2 — Steam Link Terminal

**Status: deferred until Phase 1 passes on hardware.**

## Goal

Use the Cardputer ADV as a tiny display/controller terminal for a real gaming PC, with Steam Link doing the heavy video decoding on the iPhone.

## Proposed path

```text
Gaming PC
   |
Steam Remote Play
   v
iPhone running Steam Link
   |
ReplayKit broadcast
   v
Cardputer ADV display
```

## Required change from Phase 1

The iPhone cannot be isolated on the Cardputer-only access point if Steam Link needs to reach the PC over the normal LAN. Therefore Phase 2 needs **shared-LAN mode**:

- Cardputer joins home Wi-Fi.
- iPhone remains on the same home Wi-Fi.
- Cardputer advertises or exposes its receiver address.
- iPhone sender discovers/configures that address.

## Input return path options

Evaluate after video works:

1. Cardputer as BLE HID keyboard/gamepad paired to iPhone.
2. Cardputer sends input events over LAN to a small PC companion service.
3. Hybrid mode: normal keyboard keys via BLE HID, advanced controls via network.

## Not planned

Do not attempt to port the complete Steam Link client to ESP32-S3 as the primary architecture.
