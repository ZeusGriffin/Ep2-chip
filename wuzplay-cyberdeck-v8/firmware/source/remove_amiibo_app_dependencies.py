#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path("pixljs-source").resolve()
SETTINGS_H = ROOT / "fw/application/src/mod/settings.h"

if not SETTINGS_H.exists():
    raise SystemExit(f"settings header not found: {SETTINGS_H}")

text = SETTINGS_H.read_text()
text = text.replace('#include "app_amiibolink.h"', '#include "ble_amiibolink.h"')
SETTINGS_H.write_text(text)

verify = SETTINGS_H.read_text()
if 'app_amiibolink.h' in verify:
    raise SystemExit("settings still depends on removed AmiiboLink app header")
if '#include "ble_amiibolink.h"' not in verify:
    raise SystemExit("BLE compatibility types are not available after cleanup")

print("Removed settings dependency on app/amiibolink while preserving settings layout")
