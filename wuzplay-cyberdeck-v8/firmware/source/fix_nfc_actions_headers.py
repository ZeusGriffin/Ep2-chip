#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path("pixljs-source").resolve()
HDR = ROOT / "fw/application/src/app/nfc_actions/app_nfc_actions.h"
SRC = ROOT / "fw/application/src/app/nfc_actions/app_nfc_actions.c"
if not HDR.exists() or not SRC.exists():
    raise SystemExit("NFC Actions source/header not found")

text = HDR.read_text()
if '#include "mui_list_view.h"' not in text:
    text = text.replace('#include "mui_include.h"', '#include "mui_include.h"\n#include "mui_list_view.h"', 1)
HDR.write_text(text)

# Use the same stable Siji glyph codepoints already used by the native UI rather
# than relying on ICON_* aliases that were supplied indirectly by removed apps.
src = SRC.read_text()
src = src.replace('item->icon == ICON_BACK', 'item->icon == 0xe069')
src = src.replace('mui_list_view_add_item(app->p_list_view, ICON_BACK, "Back", NULL);',
                  'mui_list_view_add_item(app->p_list_view, 0xe069, "Back", NULL);')
src = src.replace('mui_list_view_add_item(app->p_list_view, ICON_FILE, actions[i].label, (void *)&actions[i]);',
                  'mui_list_view_add_item(app->p_list_view, 0xe146, actions[i].label, (void *)&actions[i]);')
SRC.write_text(src)

if '#include "mui_list_view.h"' not in HDR.read_text():
    raise SystemExit("mui_list_view.h was not added")
verify = SRC.read_text()
if 'ICON_BACK' in verify or 'ICON_FILE' in verify:
    raise SystemExit("legacy icon aliases still present")
if '0xe069' not in verify or '0xe146' not in verify:
    raise SystemExit("native UI glyphs were not applied")

print("NFC Actions UI dependencies made explicit")
print("Back/File icons use native stable glyph codepoints")
