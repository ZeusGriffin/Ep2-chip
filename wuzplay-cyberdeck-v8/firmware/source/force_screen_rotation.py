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

# Hardware-photo correction (v804):
# v803 forced U8G2_MIRROR_VERTICAL and the physical LCD photo proved that this
# flips the entire rendered UI top-to-bottom. Remove that transform and use the
# LCD's native U8G2_R0 orientation instead. This fixes only display coordinates;
# all apps/features remain unchanged.
#
# Keep the result independent of persisted display_flip state so an old setting
# cannot reintroduce the bad orientation at boot or from the Settings callback.
old_normal = "static const u8g2_cb_t *mui_u8g2_get_rotation_cb(bool flipped) { return flipped ? U8G2_R2 : U8G2_R0; }"
old_rot2 = "static const u8g2_cb_t *mui_u8g2_get_rotation_cb(bool flipped) { (void)flipped; return U8G2_R2; }"
old_mirror_vertical = "static const u8g2_cb_t *mui_u8g2_get_rotation_cb(bool flipped) { (void)flipped; return U8G2_MIRROR_VERTICAL; }"
new = "static const u8g2_cb_t *mui_u8g2_get_rotation_cb(bool flipped) { (void)flipped; return U8G2_R0; }"

for old in (old_normal, old_rot2, old_mirror_vertical):
    if old in text:
        text = text.replace(old, new)
        break
else:
    if new not in text:
        raise SystemExit("expected display rotation helper not found")

PATH.write_text(text)

verify = PATH.read_text()
if '#include "mui_core.h"' not in verify:
    raise SystemExit("mui_core.h dependency was not added")
if "(void)flipped; return U8G2_R0;" not in verify:
    raise SystemExit("native upright U8G2_R0 orientation was not applied")
if "return U8G2_MIRROR_VERTICAL;" in verify:
    raise SystemExit("old vertical mirror transform is still present")

print("Wuzplay v804 LCD orientation forced to native upright U8G2_R0")
print("Removed the v803 top-to-bottom mirror proven wrong by hardware photo")
print("MUI rotation dependencies made explicit")
