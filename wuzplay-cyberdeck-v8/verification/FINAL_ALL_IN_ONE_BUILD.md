# Wuzplay v8 All-in-One — Verified Build

Build workflow run: `32611772868`
Build head: `4589e2f2f3d1fb3c06b6816f935ee37b10f44537`
Board: `LCD`
Application version: `800`

## Installable Nordic DFU

File: `01_INSTALL_WUZPLAY_V8_ALL_IN_ONE_DFU_KEEP_ZIPPED.zip`
Size: `335231` bytes
SHA-256: `a92d5e9b2d5080968d56b726f8aa745848c19c8ac6b36f527f3dcc9cd2141631`

DFU members:
- `manifest.json` — 137 bytes
- `pixljs.dat` — 142 bytes
- `pixljs.bin` — 334636 bytes

`pixljs.bin` SHA-256: `408f821524942fe1b27198d61eb69c6fc4ae2869afd55263d15652d9ac9d770e`

## Build gates passed

- Exact custom PR #457 source snapshot fetched and SHA-pinned.
- Wuzplay v8 patches applied.
- Compact tech/pixel UI applied.
- Release app flags forced on.
- LCD firmware compiled successfully.
- Nordic DFU generated successfully.
- ELF symbol gate passed for NFC Actions, NFC Cards, Media Player, Cyber Arcade, Wireless Files, all four historic games, Cyber Hoops 2K, and Cyber Fighter.
- Binary marker gate passed for all 19 built-in NFC actions and all six arcade games.
- DFU ZIP integrity and manifest member mapping passed.

## Final visible layout

- NFC Actions — 19 built-in iPhone/CyberSync/Govee actions; no .bin import required.
- NFC Cards — Chameleon/card emulation and advanced card tools.
- Media Player — built-in player/file browser.
- Cyber Arcade — Arkanoid, Invaders, Lander, Tris, Cyber Hoops 2K, Cyber Fighter.
- Wireless Files — BLE file transfer.
- System — display, Back/Return, power, storage, DFU and other settings.

Amiibo Database and AmiiboLink are not registered in the final menu. The legacy NTAG application slot is repurposed as NFC Actions.

## Physical verification

CI/build verification is complete. A physical-device flash and button/NFC smoke test is still required before calling the release hardware-verified.
