#!/usr/bin/env python3
"""Import a supplied gold Wuzplay v8 DFU only if its exact SHA-256 matches."""
from __future__ import annotations
import argparse, hashlib, shutil
from pathlib import Path

EXPECTED = "bd383930bed88376c3c39e839d46ca513d2b5f61c1b8cb63233ce4d1ce1aeea6"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("dfu", type=Path)
    ap.add_argument("--root", type=Path, default=Path("."))
    args = ap.parse_args()
    raw = args.dfu.read_bytes()
    got = hashlib.sha256(raw).hexdigest()
    if got != EXPECTED:
        raise SystemExit(f"Refusing DFU: expected {EXPECTED}, got {got}")
    out = args.root / "firmware/verified/01_INSTALL_WUZPLAY_DFU_KEEP_ZIPPED.zip"
    out.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(args.dfu, out)
    print(f"imported verified DFU -> {out} sha256={got}")


if __name__ == "__main__":
    main()
