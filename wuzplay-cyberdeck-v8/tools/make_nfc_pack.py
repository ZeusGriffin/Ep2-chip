#!/usr/bin/env python3
"""Build the Wuzplay Cyberdeck v8 companion NFC pack.

The generated files are 540-byte NTAG215-style raw dumps containing a single
NDEF URI record. They are companion tags for Card Emulator / Tag Explorer,
not Nordic DFU firmware.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import tempfile
from pathlib import Path
from urllib.parse import quote

TAG_SIZE = 540
CC = bytes([0xE1, 0x10, 0x3E, 0x00])


def shortcut(name: str) -> str:
    return "shortcuts://run-shortcut?name=" + quote(name, safe="")


def scriptable(script: str, mode: str | None = None) -> str:
    uri = "scriptable:///run?scriptName=" + quote(script, safe="")
    if mode:
        uri += "&mode=" + quote(mode, safe="")
    return uri


PRESETS = {
    "cybersync_system.bin": {"title": "CyberSync System", "uri": scriptable("CyberSync", "system")},
    "cybersync_dashboard.bin": {"title": "CyberSync Dashboard", "uri": scriptable("CyberSync", "dashboard")},
    "cybersync_network.bin": {"title": "CyberSync Network", "uri": scriptable("CyberSync", "network")},
    "cybersync_tools.bin": {"title": "CyberSync Tools", "uri": scriptable("CyberSync", "tools")},
    "cybersync_alerts.bin": {"title": "CyberSync Alerts", "uri": scriptable("CyberSync", "alerts")},
    "cybersync_all.bin": {"title": "CyberSync All", "uri": scriptable("CyberSync", "all")},
    "open_wuzsync.bin": {"title": "Open WuzSync", "uri": scriptable("WuzSync")},
    "drive_home.bin": {"title": "Drive Home", "uri": shortcut("Drive Home")},
    "find_car.bin": {"title": "Find Car", "uri": shortcut("Find Car")},
    "flashlight.bin": {"title": "Flashlight", "uri": shortcut("Flashlight")},
    "quick_note.bin": {"title": "Quick Note", "uri": shortcut("Quick Note")},
    "timer_10m.bin": {"title": "10-Minute Timer", "uri": shortcut("10 Minute Timer")},
    "govee_on.bin": {"title": "Govee On", "uri": shortcut("Govee On")},
    "govee_off.bin": {"title": "Govee Off", "uri": shortcut("Govee Off")},
    "govee_bright.bin": {"title": "Govee Bright", "uri": shortcut("Govee Bright")},
    "govee_relax.bin": {"title": "Govee Relax", "uri": shortcut("Govee Relax")},
    "govee_movie.bin": {"title": "Govee Movie", "uri": shortcut("Govee Movie")},
    "govee_red.bin": {"title": "Govee Red", "uri": shortcut("Govee Red")},
    "govee_blue.bin": {"title": "Govee Blue", "uri": shortcut("Govee Blue")},
}


def uid_for(name: str) -> bytes:
    digest = hashlib.sha256(("WUZPLAY-V8:" + name).encode()).digest()
    return bytes([0x04]) + digest[:6]


def make_ndef_uri(uri: str) -> bytes:
    payload = b"\x00" + uri.encode("utf-8")
    if len(payload) > 255:
        raise ValueError("URI too long for short NDEF record")
    return bytes([0xD1, 0x01, len(payload), 0x55]) + payload


def make_dump(name: str, uri: str) -> bytes:
    uid = uid_for(name)
    tag = bytearray([0x00] * TAG_SIZE)
    tag[0:4] = bytes([uid[0], uid[1], uid[2], 0x88 ^ uid[0] ^ uid[1] ^ uid[2]])
    tag[4:8] = uid[3:7]
    tag[8:12] = bytes([uid[3] ^ uid[4] ^ uid[5] ^ uid[6], 0x48, 0x00, 0x00])
    tag[12:16] = CC
    ndef = make_ndef_uri(uri)
    if len(ndef) < 0xFF:
        tlv = bytes([0x03, len(ndef)]) + ndef + bytes([0xFE])
    else:
        tlv = bytes([0x03, 0xFF]) + len(ndef).to_bytes(2, "big") + ndef + bytes([0xFE])
    if 16 + len(tlv) > 520:
        raise ValueError("NDEF payload does not fit NTAG215 user memory")
    tag[16 : 16 + len(tlv)] = tlv
    return bytes(tag)


def build(out: Path) -> dict:
    out.mkdir(parents=True, exist_ok=True)
    items = []
    for filename, meta in PRESETS.items():
        data = make_dump(filename, meta["uri"])
        (out / filename).write_bytes(data)
        items.append({"file": filename, "title": meta["title"], "uri": meta["uri"], "size": len(data), "sha256": hashlib.sha256(data).hexdigest()})
    manifest = {"package": "Wuzplay Cyberdeck v8 NFC Pack", "format": "540-byte NTAG215-style raw dump containing NDEF URI", "count": len(items), "important": "Companion NFC presets; not DFU firmware.", "presets": items}
    (out / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return manifest


def check(out: Path) -> int:
    with tempfile.TemporaryDirectory() as td:
        expected = Path(td)
        build(expected)
        names = sorted(p.name for p in expected.iterdir())
        actual_names = sorted(p.name for p in out.iterdir() if p.is_file() and p.name != "README.md")
        if names != actual_names:
            print("NFC pack file set mismatch"); print("expected:", names); print("actual:  ", actual_names); return 1
        for name in names:
            if (expected / name).read_bytes() != (out / name).read_bytes():
                print("NFC pack mismatch:", name); return 1
    print(f"NFC pack check passed: {len(PRESETS)} presets")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(); ap.add_argument("--out", type=Path, required=True); ap.add_argument("--check", action="store_true"); args = ap.parse_args()
    return check(args.out) if args.check else (build(args.out) and 0)


if __name__ == "__main__":
    raise SystemExit(main())
