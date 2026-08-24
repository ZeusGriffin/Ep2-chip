#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path("pixljs-source").resolve()
SETTINGS = ROOT / "fw/application/src/mod/settings.c"
SCENE = ROOT / "fw/application/src/app/settings/scene/settings_scene_lcd_backlight.c"

for path in (SETTINGS, SCENE):
    if not path.exists():
        raise SystemExit(f"required LCD backlight source not found: {path}")

settings = SETTINGS.read_text()
scene = SCENE.read_text()

# The custom source snapshot shipped LCD brightness with a default of 0, which
# leaves the LCD readable only from ambient light and looks permanently dim.
# Make a fresh/reset install start at full brightness.
old_default = ".lcd_backlight = 0,"
new_default = ".lcd_backlight = 100,"
if old_default in settings:
    settings = settings.replace(old_default, new_default, 1)
elif new_default not in settings:
    raise SystemExit("lcd_backlight default initializer not found")

# Migrate existing persisted 0 brightness from older firmware to 100. Keep the
# rest of the valid user-selected range intact. Values outside 0..100 also fall
# back to 100 instead of the old zero-brightness fallback.
old_validate = "INT8_VALIDATE(m_settings_data.lcd_backlight, 0, 100, 0);"
new_validate = "if (m_settings_data.lcd_backlight == 0 || m_settings_data.lcd_backlight > 100) { m_settings_data.lcd_backlight = 100; }"
if old_validate in settings:
    settings = settings.replace(old_validate, new_validate, 1)
elif new_validate not in settings:
    raise SystemExit("lcd_backlight validation line not found")

# Prevent the settings slider from saving a zero/off backlight again. 10 keeps a
# low-brightness option while ensuring the LCD remains visibly backlit.
old_min = "mui_progress_bar_set_min_value(app->p_progress_bar, 0);"
new_min = "mui_progress_bar_set_min_value(app->p_progress_bar, 10);"
if old_min in scene:
    scene = scene.replace(old_min, new_min, 1)
elif new_min not in scene:
    raise SystemExit("LCD backlight progress minimum not found")

SETTINGS.write_text(settings)
SCENE.write_text(scene)

verify_settings = SETTINGS.read_text()
verify_scene = SCENE.read_text()
checks = [
    (".lcd_backlight = 100," in verify_settings, "default brightness not set to 100"),
    (new_validate in verify_settings, "zero/invalid brightness migration not installed"),
    (new_min in verify_scene, "brightness slider minimum not raised to 10"),
    ("INT8_VALIDATE(m_settings_data.lcd_backlight, 0, 100, 0);" not in verify_settings,
     "old zero fallback still present"),
]
for ok, msg in checks:
    if not ok:
        raise SystemExit(msg)

print("Wuzplay LCD backlight fix applied")
print("Default/reset brightness: 100%")
print("Persisted 0/invalid brightness migrates to 100%")
print("User brightness range: 10-100% with Back/Cancel restore behavior preserved")
