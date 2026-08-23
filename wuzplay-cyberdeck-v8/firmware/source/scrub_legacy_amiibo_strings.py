#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path("pixljs-source").resolve()
SRC = ROOT / "fw/application/src"
if not SRC.exists():
    raise SystemExit(f"source directory not found: {SRC}")

# Keep protocol/function/type identifiers intact (ble_amiibolink_*, enum names,
# wire compatibility). Only rewrite human-readable C string literals so the UI,
# logs and compiled text no longer expose the legacy AmiiboLink label.
string_re = re.compile(r'"(?:\\.|[^"\\])*"')
changed_files = []
changed_literals = 0

for path in list(SRC.rglob("*.c")) + list(SRC.rglob("*.h")):
    text = path.read_text(errors="ignore")

    def repl(match):
        nonlocal_count = None
        literal = match.group(0)
        if "AmiiboLink" not in literal:
            return literal
        return literal.replace("AmiiboLink", "NFC Link")

    new_text = string_re.sub(repl, text)
    if new_text != text:
        changed_literals += text.count("AmiiboLink") - new_text.count("AmiiboLink")
        path.write_text(new_text)
        changed_files.append(path.relative_to(SRC).as_posix())

# Hard check only on quoted literals. Lowercase internal protocol identifiers such
# as ble_amiibolink_* are intentionally preserved for compatibility.
remaining = []
for path in list(SRC.rglob("*.c")) + list(SRC.rglob("*.h")):
    text = path.read_text(errors="ignore")
    for m in string_re.finditer(text):
        if "AmiiboLink" in m.group(0):
            remaining.append(f"{path.relative_to(SRC)}: {m.group(0)}")
if remaining:
    raise SystemExit("legacy AmiiboLink string literal remains:\n" + "\n".join(remaining[:20]))

print(f"Scrubbed {changed_literals} legacy AmiiboLink text literal(s) in {len(changed_files)} file(s)")
print("BLE protocol identifiers left intact for compatibility")
