# Wuzplay Cyberdeck v8 NFC pack

Generate the 19 companion `.bin` presets with:

```bash
python3 ../tools/make_nfc_pack.py --out .
python3 ../tools/make_nfc_pack.py --out . --check
```

The generated pack contains:

- CyberSync System, Dashboard, Network, Tools, Alerts, All
- Open WuzSync
- Drive Home, Find Car, Flashlight, Quick Note, 10-Minute Timer
- Govee On, Off, Bright, Relax, Movie, Red, Blue

These are 540-byte NTAG215-style NDEF URI dumps for Card Emulator / Tag Explorer. They are **not DFU firmware**.
