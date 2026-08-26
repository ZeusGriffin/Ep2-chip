# WuzHack / Wuzplay Cyberdeck v8 — Project History and Source of Truth

This document consolidates the project decisions, firmware lineage, custom-feature history, build verification, and currently known ChatGPT-derived requirements for the Wuzplay Cyberdeck v8 work.

It is intended to make this repository the durable technical source of truth for the project.

## Scope

The repository is authoritative for:

- firmware source/reconstruction scripts
- custom patch scripts
- build workflows
- DFU metadata and hashes
- verification reports
- capability documentation
- companion/NFC tooling
- important project decisions that can be recovered from the WurzHack project context

ChatGPT conversation transcripts are not automatically mirrored to GitHub. This file records the technically relevant decisions currently available to the project, but it is not a verbatim export of every historical chat.

## Project decisions recovered from WurzHack chat context

1. Treat the WurzHack project as one continuous firmware project rather than a set of unrelated chat experiments.
2. Use one master firmware line and merge compatible custom changes into a single DFU.
3. Fix the screen orientation first and preserve that behavior while adding the rest of the custom firmware.
4. Preserve Back/Return navigation behavior.
5. Prefer the newest explicit requirement when an older and newer project instruction conflict.
6. Inspect and verify the code/build instead of assuming historical source snapshots reproduce a known binary.
7. Keep firmware behavior separate from companion-side behavior and clearly label what is proven versus reconstructed.
8. Build a final all-in-one DFU rather than requiring multiple sequential firmware flashes for different features.
9. Keep reproducible hashes, build records, verification gates, and CI evidence in the repository.
10. Use this repository as the long-term technical source of truth for the combined firmware work.

## Firmware lineage

### Known-good binary anchor

The original reconstruction began from a known-good Wuzplay v8 Nordic DFU instead of assuming that an unrelated stock source tree reproduced it.

Known gold `pixljs.bin` SHA-256:

`722942a6a12f9851736c204932300d66bf231b2b425c4c5c873cf0b0da4041ad`

The exact original source tree that produced that binary has not been recovered.

### Historical custom source reference points

The reconstruction tracks these historical custom pixl.js references:

- combined flip-screen + return-key snapshot: `74a505a948a1cfedd296b0033239f74b4090c420`
- later return-key-only head: `63379c3c4c803fecc59f3a12f3a7f6b2105e12aa`
- later flip-screen head: `0a9069a1d2e81bf713f8cf73b854c07ef8b48084`

These are reference points only; none is claimed to be the exact source for the original gold v8 binary.

### v802 all-in-one

Commit `4971f8d1378d3d01d1a4d0eff99f35dfdf7a0d27` records the Wuzplay v8 all-in-one ROT2 firmware line.

That line merged:

- forced screen rotation
- standalone NFC Actions outside legacy Amiibo app folders
- six-game Cyber Arcade
- Media Player
- NFC Cards / Chameleon functionality
- Wireless Files
- Back support
- compact tech UI
- all 19 built-in NFC actions

### v803 orientation-corrected all-in-one

The v803 line corrected the orientation implementation to the exact semantics of:

`g.setRotation(2, true)`

At the firmware layer this is implemented as:

`U8G2_MIRROR_VERTICAL`

The helper intentionally returns the same transform regardless of persisted `display_flip` state so old settings cannot reverse the intended startup orientation.

Key commits:

- `403e9fdfeb2a26ab290f103efeda59b15bc73be4` — build v803 with exact rotation 2 plus reflect orientation
- `cc3fa19088bad5d5accdca4abcbedaece5ae648b` — fix orientation to `setRotation(2,true)` semantics; CI passed
- `13aa2fc20d37a247327bc0c0b9cfba6f45b4df63` — record verified v803 rotation2-reflect DFU build

Recorded v803 DFU:

`01_INSTALL_WUZPLAY_V8_ALL_IN_ONE_ROT2_REFLECT_DFU_KEEP_ZIPPED.zip`

DFU SHA-256:

`ceba40f45a4c136a506a6fed18d7f22fb4697332792aeab71e67afa300aae1fc`

Recorded `pixljs.bin` size:

`268004` bytes

## Final all-in-one custom firmware capability set

### 1. Screen orientation fix

Exact `g.setRotation(2, true)` behavior is forced at the display callback layer using `U8G2_MIRROR_VERTICAL` semantics.

### 2. Back / Return navigation

Back/Return support is retained so navigation can return to the previous page instead of trapping the user in forward-only menu flows.

Target button order documented for the custom v8 package:

`Left / Select / Right / Back`

### 3. Compact Cyberdeck UI

The release applies the compact tech/pixel Cyberdeck interface instead of leaving the stock/default presentation unchanged.

### 4. NFC Actions — 19 built-in actions

The all-in-one firmware integrates the NFC Actions feature directly rather than requiring separate `.bin` imports for these built-ins.

CyberSync:

- System
- Dashboard
- Network
- Tools
- Alerts
- All

WuzSync:

- Open WuzSync

iPhone workflows:

- Drive Home
- Find Car
- Flashlight
- Quick Note
- 10-Minute Timer

Govee:

- On
- Off
- Bright
- Relax
- Movie
- Red
- Blue

### 5. NFC Cards / Chameleon

The firmware retains general card emulation and advanced NFC-card tooling under NFC Cards / Chameleon-oriented functionality rather than exposing legacy Amiibo-branded menu entries.

### 6. Cyber Arcade — 6 games

- Arkanoid
- Invaders
- Lander
- Tris
- Cyber Hoops 2K
- Cyber Fighter

### 7. Media Player

Built-in media/video player and file-browser functionality.

### 8. Wireless Files

BLE-based wireless file transfer.

### 9. System functionality

The integrated System area includes the relevant display, Back/Return, power, storage, DFU/firmware-update, and related settings/features available in the all-in-one line.

## Legacy Amiibo cleanup

The final all-in-one menu intentionally does not register:

- Amiibo Database
- AmiiboLink

The legacy NTAG application slot is repurposed for NFC Actions. Historical cleanup commits also remove inherited Amiibo app dependencies and scrub/rename legacy Amiibo labels while preserving the useful underlying NFC protocol/tooling where needed.

## Verification status

For the recorded v803 build, the following checks are recorded as passed:

- source patch
- static feature gate
- LCD compile
- linked-symbol checks
- compiled-feature scan
- Nordic DFU member validation
- ZIP CRC/integrity
- artifact packaging
- local recheck

The verification record lists the physical screen test as pending. A real-device flash plus button/NFC smoke test is therefore still required before describing v803 as physically hardware-verified.

## Repository structure relevant to this history

- `firmware/source/` — reconstruction and patch scripts
- `firmware/verified/` — verified/anchored firmware material available in the repository packaging flow
- `verification/` — build records, hashes, and verification reports
- `docs/CAPABILITIES.md` — capability map
- `docs/INSTALL.md` — installation notes
- `companion/` — CyberSync/WuzSync companion material
- `nfc_pack/` — generated NFC companion presets
- `tools/` — deterministic generation/import/verification tooling
- `.github/workflows/` — CI/build workflows

## Source-of-truth rule going forward

When project chat instructions and repository history need to be reconciled:

1. Preserve known-good artifacts and hashes.
2. Prefer newer explicit project requirements over older conflicting ones.
3. Do not claim an uncertain historical source tree produced a binary unless proven.
4. Record each final firmware change in Git history.
5. Record DFU hashes and verification results in `verification/`.
6. Update this document when a major project decision changes the intended final firmware.
7. Keep unverified hardware behavior clearly marked until tested on the physical device.

## Chat-history limitation

This repository now contains a consolidated technical record of the WurzHack/Wuzplay work that is currently recoverable from the project context and Git history. It does **not** contain a verbatim archive of every ChatGPT message ever exchanged in the project, because ChatGPT project conversations are not automatically exported to GitHub.

If additional historical chat exports become available, their technically relevant requirements and decisions should be merged into this document (or stored in a dedicated `docs/chat-history/` archive) without overwriting the verified firmware history above.
