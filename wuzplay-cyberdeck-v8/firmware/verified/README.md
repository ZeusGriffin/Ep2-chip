# Verified v8 DFU anchor

The known-good DFU is `01_INSTALL_WUZPLAY_DFU_KEEP_ZIPPED.zip`.

Archive SHA-256:
`bd383930bed88376c3c39e839d46ca513d2b5f61c1b8cb63233ce4d1ce1aeea6`

Gold `pixljs.bin` SHA-256:
`722942a6a12f9851736c204932300d66bf231b2b425c4c5c873cf0b0da4041ad`

The complete pre-verified package supplied from the reconstruction contains the exact DFU. The connected GitHub writer was not byte-safe for the 365 KB binary blob, so the source branch intentionally does not substitute a corrupted copy.

To place a DFU in this directory, use:

```bash
python3 ../../tools/import_verified_dfu.py /path/to/01_INSTALL_WUZPLAY_DFU_KEEP_ZIPPED.zip --root ../..
```

The importer refuses every archive whose SHA-256 does not match the gold hash.
