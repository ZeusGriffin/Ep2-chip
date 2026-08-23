# Wuzplay Cyberdeck v8 — Preverified Reconstruction

This directory reconstructs the newest known Wuzplay v8 package using an artifact-first Karpathy workflow: preserve the known-good DFU byte-for-byte, verify it, keep NFC/phone companion features separate from compiled firmware, and only claim capabilities that can be verified.

## Gold DFU anchor

- Installer: `01_INSTALL_WUZPLAY_DFU_KEEP_ZIPPED.zip`
- `pixljs.bin`: 372364 bytes
- SHA-256: `722942a6a12f9851736c204932300d66bf231b2b425c4c5c873cf0b0da4041ad`
- `pixljs.dat`: 141 bytes
- SHA-256: `ba62b0c53e158928c5573e85b9850bd7e8d005c7709a68c7c8221880d37a98d3`
- `manifest.json`: 137 bytes
- SHA-256: `089c4e8e2d2f0958da9e0dd559bf14b1be6f05dd7685efa6a938dc6f5aba97fc`

## Package layers

- `firmware/verified/` — immutable verified DFU reference.
- `firmware/source/` — source-reconstruction workspace; do not claim reproducible source until it builds and hardware-tests.
- `nfc_pack/` — 19 NFC companion presets, not DFU firmware.
- `companion/` — CyberSync/WuzSync and Govee Shortcut setup.
- `verification/` — machine-readable verification results.
- `tools/` — reconstruction/verification utilities.

## Verified preflight

The local reconstruction passed ZIP integrity, required-member, manifest mapping, exact DFU size/hash, custom binary-marker, 19/19 NFC NDEF dump, companion JavaScript syntax, and documentation checks.

Hardware flashing has **not** been performed in this chat environment, so this is preverified rather than hardware-certified.

## Button order

1. Left
2. Select / OK
3. Right
4. Back

Back is intended to return to the exact previous page.

## Safety rule

NFC preset `.bin` files are companion tag dumps. Never upload them through Firmware Upgrade. Only the Nordic DFU ZIP belongs in Firmware Upgrade.
