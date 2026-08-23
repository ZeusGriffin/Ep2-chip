#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path("pixljs-source").resolve()
PATH = ROOT / "fw/application/src/mui/mui_u8g2.c"

if not PATH.exists():
    raise SystemExit(f"display source not found: {PATH}")

text = PATH.read_text()
old = "static const u8g2_cb_t *mui_u8g2_get_rotation_cb(bool flipped) { return flipped ? U8G2_R2 : U8G2_R0; }"
new = "static const u8g2_cb_t *mui_u8g2_get_rotation_cb(bool flipped) { (void)flipped; return U8G2_R2; }"

if old in text:
    text = text.replace(old, new)
elif new not in text:
    raise SystemExit("expected display rotation helper not found")

PATH.write_text(text)

# Hard verification: this build must boot at rotation 2 (180 degrees), independent
# of persisted display_flip state. This is the firmware-layer equivalent of the
# proven graphics fix: g.setRotation(2, true).
verify = PATH.read_text()
if "(void)flipped; return U8G2_R2;" not in verify:
    raise SystemExit("forced rotation 2 was not applied")

print("Wuzplay display orientation forced to rotation 2 / U8G2_R2")
