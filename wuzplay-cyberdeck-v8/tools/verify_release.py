#!/usr/bin/env python3
from __future__ import annotations
import argparse, hashlib, json, zipfile
from pathlib import Path

EXPECTED = {
    "manifest.json": (137, "089c4e8e2d2f0958da9e0dd559bf14b1be6f05dd7685efa6a938dc6f5aba97fc"),
    "pixljs.bin": (372364, "722942a6a12f9851736c204932300d66bf231b2b425c4c5c873cf0b0da4041ad"),
    "pixljs.dat": (141, "ba62b0c53e158928c5573e85b9850bd7e8d005c7709a68c7c8221880d37a98d3"),
}
ARCHIVE_SHA = "bd383930bed88376c3c39e839d46ca513d2b5f61c1b8cb63233ce4d1ce1aeea6"
BINARY_MARKERS = [b"Cyberdeck", b"Cyber Dashboard", b"Cyber Tools", b"Back to Cyberdeck", b"Cyber Shadow", b"Arkanoid", b"Invaders", b"Lander", b"Tris", b"Card Emulator", b"Amiibo Emulator", b"Amiibo Database", b"AmiiboLink", b"BLE File Transfer", b"Video Player", b"Firmware Update"]

def sha(b: bytes) -> str: return hashlib.sha256(b).hexdigest()

def decode_ndef_uri(tag: bytes) -> str:
    if len(tag) != 540: raise ValueError(f"expected 540-byte dump, got {len(tag)}")
    if tag[12:16] != bytes([0xE1,0x10,0x3E,0x00]): raise ValueError("invalid NTAG215 capability container")
    p=16
    if tag[p] != 0x03: raise ValueError("missing NDEF TLV")
    p+=1; ln=tag[p]; p+=1
    if ln == 0xFF: ln=int.from_bytes(tag[p:p+2],"big"); p+=2
    ndef=tag[p:p+ln]
    if len(ndef) < 5 or ndef[0] != 0xD1 or ndef[1] != 0x01 or ndef[3] != 0x55 or ndef[4] != 0x00: raise ValueError("unsupported NDEF URI record")
    return ndef[5:].decode("utf-8")

def main() -> int:
    ap=argparse.ArgumentParser(); ap.add_argument("--root", type=Path, required=True); ap.add_argument("--write-report", action="store_true"); ap.add_argument("--allow-missing-dfu", action="store_true"); args=ap.parse_args(); root=args.root
    report={"release":"Wuzplay Cyberdeck v8","passed":True,"checks":[],"nfc":[]}
    def ok(name, detail="passed"): report["checks"].append({"check":name,"status":"passed","detail":detail})
    def fail(name, detail): report["passed"]=False; report["checks"].append({"check":name,"status":"failed","detail":detail})

    dfu=root/"firmware/verified/01_INSTALL_WUZPLAY_DFU_KEEP_ZIPPED.zip"
    if not dfu.exists():
        if args.allow_missing_dfu: ok("dfu repository transport", "gold DFU omitted from connector branch; use tools/import_verified_dfu.py with the exact hash")
        else: fail("dfu archive", "missing")
    else:
        raw=dfu.read_bytes()
        if sha(raw)==ARCHIVE_SHA: ok("dfu archive sha256", ARCHIVE_SHA)
        else: fail("dfu archive sha256", sha(raw))
        try:
            with zipfile.ZipFile(dfu) as z:
                bad=z.testzip()
                if bad is None: ok("zip integrity")
                else: fail("zip integrity", bad)
                names=set(z.namelist())
                if set(EXPECTED)==names: ok("dfu members", ", ".join(sorted(names)))
                else: fail("dfu members", repr(sorted(names)))
                members={n:z.read(n) for n in EXPECTED}
                for n,(size,h) in EXPECTED.items():
                    if len(members[n])==size and sha(members[n])==h: ok(f"{n} size/hash", f"{size} bytes {h}")
                    else: fail(f"{n} size/hash", f"size={len(members[n])} sha256={sha(members[n])}")
                app=json.loads(members["manifest.json"]).get("manifest",{}).get("application",{})
                if app.get("bin_file")=="pixljs.bin" and app.get("dat_file")=="pixljs.dat": ok("manifest mapping")
                else: fail("manifest mapping", repr(app))
                missing=[x.decode() for x in BINARY_MARKERS if x not in members["pixljs.bin"]]
                if not missing: ok("custom binary markers", f"{len(BINARY_MARKERS)} markers")
                else: fail("custom binary markers", ", ".join(missing))
                if b"Govee" not in members["pixljs.bin"]: ok("Govee separation", "Govee remains in companion/NFC layer, not the gold DFU binary")
                else: fail("Govee separation", "unexpected Govee string in gold DFU")
        except Exception as e: fail("dfu parse", repr(e))

    manifest=json.loads((root/"nfc_pack/manifest.json").read_text())
    if manifest.get("count")==19 and len(manifest.get("presets",[]))==19: ok("nfc preset count", "19")
    else: fail("nfc preset count", repr(manifest.get("count")))
    for item in manifest.get("presets",[]):
        p=root/"nfc_pack"/item["file"]
        try:
            b=p.read_bytes(); uri=decode_ndef_uri(b); good=len(b)==item["size"]==540 and sha(b)==item["sha256"] and uri==item["uri"]
            report["nfc"].append({"file":item["file"],"passed":good,"sha256":sha(b),"uri":uri})
            if not good: report["passed"]=False
        except Exception as e: report["nfc"].append({"file":item["file"],"passed":False,"error":repr(e)}); report["passed"]=False
    if report["nfc"] and all(x.get("passed") for x in report["nfc"]): ok("nfc dump validation", "19/19 valid NDEF NTAG215-style dumps")
    else: fail("nfc dump validation", "one or more invalid presets")

    for rel in ["companion/CyberSync.js","companion/WuzSync.js","companion/GOVEE_SHORTCUTS.md","docs/INSTALL.md","docs/CAPABILITIES.md"]:
        if (root/rel).is_file() and (root/rel).stat().st_size>0: ok(rel)
        else: fail(rel,"missing or empty")
    if args.write_report: (root/"verification/PREVERIFY_REPORT.json").write_text(json.dumps(report,indent=2)+"\n")
    print(json.dumps(report,indent=2)); return 0 if report["passed"] else 1

if __name__=="__main__": raise SystemExit(main())
