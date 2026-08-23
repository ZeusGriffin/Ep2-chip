#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path("pixljs-source").resolve()
HDR = ROOT / "fw/application/src/app/nfc_actions/app_nfc_actions.h"
if not HDR.exists():
    raise SystemExit(f"NFC Actions header not found: {HDR}")

text = HDR.read_text()
if '#include "mui_list_view.h"' not in text:
    text = text.replace('#include "mui_include.h"', '#include "mui_include.h"\n#include "mui_list_view.h"', 1)
HDR.write_text(text)

if '#include "mui_list_view.h"' not in HDR.read_text():
    raise SystemExit("mui_list_view.h was not added")

print("NFC Actions list-view dependency made explicit")
