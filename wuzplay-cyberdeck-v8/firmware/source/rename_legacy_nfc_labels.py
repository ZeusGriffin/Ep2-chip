#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path("pixljs-source").resolve()
I18N = ROOT / "fw/application/src/i18n"
if not I18N.exists():
    raise SystemExit(f"i18n directory not found: {I18N}")

mapping = {
    "_L_APP_AMIIBO": "NFC Actions",
    "_L_APP_AMIIBOLINK": "NFC Link",
    "_L_APP_AMIIDB": "NFC Library",
}

changed = 0
for path in I18N.glob("*.c"):
    text = path.read_text(errors="ignore")
    original = text
    for key, value in mapping.items():
        text = re.sub(rf'(\[{re.escape(key)}\]\s*=\s*)"[^"]*"', rf'\1"{value}"', text)
    if text != original:
        path.write_text(text)
        changed += 1

if changed == 0:
    raise SystemExit("no legacy NFC app labels were updated")

joined = "\n".join(p.read_text(errors="ignore") for p in I18N.glob("*.c"))
for forbidden in ('[_L_APP_AMIIBOLINK] = "AmiiboLink"', '[_L_APP_AMIIDB] = "Amiibo Database"'):
    if forbidden in joined:
        raise SystemExit(f"legacy app label remains: {forbidden}")

print(f"Renamed legacy NFC app labels in {changed} language files")
print("Visible app labels: NFC Actions / NFC Link / NFC Library")
