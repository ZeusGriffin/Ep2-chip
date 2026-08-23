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

# Exact semantics of the user's proven Espruino fix:
#     g.setRotation(2, true)
# Espruino rotation 2 first sets INVERT_X | INVERT_Y. With reflect=true and no
# SWAP_XY, it toggles INVERT_X back off, leaving INVERT_Y only. U8g2 exposes
# that exact final transform as U8G2_MIRROR_VERTICAL.
#
# Force the callback regardless of persisted display_flip state. The settings
# screen can call mui_u8g2_set_display_flip(), but this helper intentionally
# returns the same proven transform every time so old settings cannot undo it.
old_normal = "static const u8g2_cb_t *mui_u8g2_get_rotation_cb(bool flipped) { return flipped ? U8G2_R2 : U8G2_R0; }"
old_rot2 = "static const u8g2_cb_t *mui_u8g2_get_rotation_cb(bool flipped) { (void)flipped; return U8G2_R2; }"
new = "static const u8g2_cb_t *mui_u8g2_get_rotation_cb(bool flipped) { (void)flipped; return U8G2_MIRROR_VERTICAL; }"

if old_normal in text:
    text = text.replace(old_normal, new)
elif old_rot2 in text:
    text = text.replace(old_rot2, new)
elif new not in text:
    raise SystemExit("expected display rotation helper not found")

PATH.write_text(text)

verify = PATH.read_text()
if '#include "mui_core.h"' not in verify:
    raise SystemExit("mui_core.h dependency was not added")
if "(void)flipped; return U8G2_MIRROR_VERTICAL;" not in verify:
    raise SystemExit("rotation 2 + reflect transform was not applied")

print("Wuzplay orientation forced to Espruino g.setRotation(2, true) semantics")
print("Final transform: U8G2_MIRROR_VERTICAL / INVERT_Y")
print("MUI rotation dependencies made explicit")
