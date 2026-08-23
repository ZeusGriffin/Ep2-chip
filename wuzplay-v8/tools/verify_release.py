#!/usr/bin/env python3
"""Verify the known-good Wuzplay Cyberdeck v8 Nordic DFU archive."""
from __future__ import annotations
import hashlib, json, sys, zipfile
from pathlib import Path

EXPECTED = {
    "manifest.json": (137, "089c4e8e2d2f0958da9e0dd559bf14b1be6f05dd7685efa6a938dc6f5aba97fc"),
    "pixljs.bin": (372364, "722942a6a12f9851736c204932300d66bf231b2b425c4c5c873cf0b0da4041ad"),
    "pixljs.dat": (141, "ba62b0c53e158928c5573e85b9850bd7e8d005c7709a68c7c8221880d37a98d3"),
}

def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()

def main() -> int:
    if len(sys.argv) != 2:
        print("usage: verify_release.py 01_INSTALL_WUZPLAY_DFU_KEEP_ZIPPED.zip")
        return 2
    archive = Path(sys.argv[1])
    with zipfile.ZipFile(archive) as z:
        bad = z.testzip()
        if bad:
            raise SystemExit(f"ZIP integrity failed at {bad}")
        names = set(z.namelist())
        if names != set(EXPECTED):
            raise SystemExit(f"unexpected DFU members: {sorted(names)}")
        for name, (size, digest) in EXPECTED.items():
            data = z.read(name)
            if len(data) != size or sha256(data) != digest:
                raise SystemExit(f"verification failed: {name}")
        manifest = json.loads(z.read("manifest.json"))
        app = manifest["manifest"]["application"]
        if app["bin_file"] != "pixljs.bin" or app["dat_file"] != "pixljs.dat":
            raise SystemExit("manifest mapping failed")
    print("Wuzplay Cyberdeck v8 DFU: VERIFIED")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
