# Wuzplay Cyberdeck v8 — reconstructed, pre-verified package

This reconstruction uses the known-good Wuzplay v8 DFU as the gold anchor and rebuilds the companion side from source. The complete packaged ZIP includes the gold DFU; the GitHub connector branch keeps the source, hashes, and importer visible because direct large-binary transport through the connector was not byte-safe.

## Working firmware anchor

Flash **only**:

`firmware/verified/01_INSTALL_WUZPLAY_DFU_KEEP_ZIPPED.zip`

Do not unzip it before using Wuzplay Firmware Upgrade.

Gold `pixljs.bin` SHA-256:

`722942a6a12f9851736c204932300d66bf231b2b425c4c5c873cf0b0da4041ad`

## What is reconstructed here

- exact verified Nordic DFU in the packaged release; SHA-checked importer for the GitHub source branch
- v8 CyberSync Scriptable companion
- v8 WuzSync Scriptable companion
- all 19 NFC companion presets, generated reproducibly from source
- seven Govee Apple Shortcut launch presets
- installation/capability documentation
- deterministic pre-verification tooling
- CI workflow to repeat the verification on GitHub

## Karpathy-style reconstruction rule

1. Start from the known-good artifact, not assumptions.
2. Separate proven firmware behavior from companion behavior.
3. Reconstruct the smallest missing pieces.
4. Make generated artifacts deterministic.
5. Verify hashes, structure, expected custom markers, and NFC payloads.
6. Keep uncertain firmware source history explicitly labeled instead of pretending stock source is the v8 source.

Run locally after importing the verified DFU:

```bash
python3 tools/import_verified_dfu.py /path/to/01_INSTALL_WUZPLAY_DFU_KEEP_ZIPPED.zip --root .
python3 tools/make_nfc_pack.py --out nfc_pack
python3 tools/make_nfc_pack.py --out nfc_pack --check
python3 tools/verify_release.py --root . --write-report
```

See `docs/INSTALL.md`, `docs/CAPABILITIES.md`, and `firmware/source/README.md`.
