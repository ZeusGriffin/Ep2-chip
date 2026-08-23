# Wuzplay Cyberdeck v8 installation

## Firmware

Use the verified file named `01_INSTALL_WUZPLAY_DFU_KEEP_ZIPPED.zip` **without unzipping it** in Wuzplay Firmware Upgrade.

Expected DFU members: `manifest.json`, `pixljs.bin`, `pixljs.dat`.

If the updater says `No manifest.json file found`, the wrong ZIP was selected.

Gold archive SHA-256: `bd383930bed88376c3c39e839d46ca513d2b5f61c1b8cb63233ce4d1ce1aeea6`.
Gold `pixljs.bin` SHA-256: `722942a6a12f9851736c204932300d66bf231b2b425c4c5c873cf0b0da4041ad`.

The complete pre-verified package supplied with this reconstruction contains that exact archive. In the GitHub source branch, import it with `tools/import_verified_dfu.py`; the importer rejects any non-matching file.

## Companion scripts

Install `companion/CyberSync.js` and `companion/WuzSync.js` in Scriptable on iPhone.

## NFC presets

Run `python3 tools/make_nfc_pack.py --out nfc_pack` to reproduce all 19 preset `.bin` files. Import desired presets through Card Emulator / Tag Explorer. Never send these through Firmware Upgrade.

## Govee

Create Apple Shortcuts using the exact names in `companion/GOVEE_SHORTCUTS.md`.

## External storage

CyberSync/WuzSync prepare text files for Wuzplay. Transfer desired files to the root of Wuzplay External Storage with MTools BLE.
