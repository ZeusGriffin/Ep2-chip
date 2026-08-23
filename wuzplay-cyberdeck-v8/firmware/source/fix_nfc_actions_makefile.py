#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path("pixljs-source").resolve()
MK = ROOT / "fw/application/Makefile"
if not MK.exists():
    raise SystemExit(f"Makefile not found: {MK}")

text = MK.read_text()

# Remove late standalone assignments emitted by the base patch. Makefile.common
# snapshots INC_FOLDERS before those lines, so late include paths do not reach CC.
text = text.replace("\nSRC_FILES += $(PROJ_DIR)/app/nfc_actions/app_nfc_actions.c\n", "\n")
text = text.replace("INC_FOLDERS += $(PROJ_DIR)/app/nfc_actions\n", "")

src_line = "  $(PROJ_DIR)/app/nfc_actions/app_nfc_actions.c \\\n"
if "$(PROJ_DIR)/app/nfc_actions/app_nfc_actions.c" not in text:
    anchor = "  $(PROJ_DIR)/app/settings/app_settings.c \\\n"
    if anchor not in text:
        raise SystemExit("SRC_FILES insertion anchor not found")
    text = text.replace(anchor, src_line + anchor, 1)

inc_line = "  $(PROJ_DIR)/app/nfc_actions \\\n"
if "$(PROJ_DIR)/app/nfc_actions \\" not in text:
    anchor = "  $(PROJ_DIR)/app/chameleon \\\n"
    if anchor not in text:
        raise SystemExit("INC_FOLDERS insertion anchor not found")
    text = text.replace(anchor, inc_line + anchor, 1)

MK.write_text(text)

verify = MK.read_text()
if verify.count("$(PROJ_DIR)/app/nfc_actions/app_nfc_actions.c") != 1:
    raise SystemExit("NFC Actions source must appear exactly once")
if verify.count("$(PROJ_DIR)/app/nfc_actions \\") != 1:
    raise SystemExit("NFC Actions include folder must appear exactly once")
if verify.index("$(PROJ_DIR)/app/nfc_actions \\") > verify.index("include $(TEMPLATE_PATH)/Makefile.common"):
    raise SystemExit("NFC Actions include folder is still too late")

print("NFC Actions source/include integrated into primary Makefile blocks")
