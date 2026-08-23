#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path("pixljs-source").resolve()
PATH = ROOT / "fw/application/src/mui/mui_u8g2.c"

if not PATH.exists():
    raise SystemExit(f"display source not found: {PATH}")

text = PATH.read_text()

# The split flip-screen patch uses mui_t/mui() in this compilation unit but relied
# on an unrelated app include path to make the type visible. Include the real MUI
# core header explicitly so removing Amiibo app folders does not break the build.
if '#include "mui_core.h"' not in text:
    text = text.replace('#include "mui_u8g2.h"', '#include "mui_u8g2.h"\n#include "mui_core.h"', 1)

# Proven orientation fix from the working build: g.setRotation(2, true).
# In this u8g2 firmware layer, rotation 2 is U8G2_R2 (180 degrees). Force it at
# the display callback so persisted display_flip settings cannot reverse startup.
old = "static const u8g2_cb_t *mui_u8g2_get_rotation_cb(bool flipped) { return flipped ? U8G2_R2 : U8G2_R0; }"
new = "static const u8g2_cb_t *mui_u8g2_get_rotation_cb(bool flipped) { (void)flipped; return U8G2_R2; }"

if old in text:
    text = text.replace(old, new)
elif new not in text:
    raise SystemExit("expected display rotation helper not found")

PATH.write_text(text)

verify = PATH.read_text()
if '#include "mui_core.h"' not in verify:
    raise SystemExit("mui_core.h dependency was not added")
if "(void)flipped; return U8G2_R2;" not in verify:
    raise SystemExit("forced rotation 2 was not applied")

print("Wuzplay display orientation forced to rotation 2 / U8G2_R2")
print("MUI rotation dependencies made explicit")
