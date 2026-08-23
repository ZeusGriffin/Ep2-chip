#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path("pixljs-source").resolve()
SRC = ROOT / "fw/application/src"

if not SRC.exists():
    raise SystemExit(f"pixl.js source tree not found: {SRC}")

# Karpathy-style reconstruction rule: start from the last verified full custom
# source snapshot, then make the smallest explicit changes needed for the final
# Wuzplay build. Do not silently inherit stock menu removals.

# Clean top-level registry. Keep the proven NFC engine but present it as
# Wuzplay NFC Actions. Hide Amiibo Database and AmiiboLink completely.
mini_app_data = r'''#include "mini_app_defines.h"
#include "app_status_bar.h"
#include "app_desktop.h"
#include "app_amiibo.h"
#include "app_ble.h"
#include "app_player.h"
#include "app_settings.h"
#include "app_chameleon.h"
#include "app_game.h"
#include <stddef.h>

const mini_app_t* mini_app_registry[] = {
    &app_status_bar_info,
    &app_desktop_info,
#ifdef APP_LEGLAMIIBO_ENABLE
    &app_amiibo_info,
#endif
    &app_chameleon_info,
#ifdef APP_PLAYER_ENABLE
    &app_player_info,
#endif
#ifdef APP_GAME_ENABLE
    &app_game_info,
#endif
#ifdef APP_LEGLAMIIBO_ENABLE
    &app_ble_info,
#endif
    &app_settings_info
};

const uint32_t mini_app_num = sizeof(mini_app_registry) / sizeof(mini_app_registry[0]);
'''
(SRC / "core/mini_app_data.c").write_text(mini_app_data)

# Better Cyberdeck labels. Apply the top-level names across every language so
# switching language never brings the removed Amiibo branding back into menus.
label_map = {
    "_L_APP_AMIIBO": "NFC Actions",
    "_L_APP_CHAMELEON": "NFC Cards",
    "_L_APP_BLE": "Wireless Files",
    "_L_APP_BLE_TITLE": "Wireless Files",
    "_L_APP_PLAYER": "Media Player",
    "_L_APP_GAME": "Cyber Arcade",
    "_L_APP_SET": "System",
}
for lang in (SRC / "i18n").glob("*.c"):
    text = lang.read_text(errors="ignore")
    original = text
    for key, value in label_map.items():
        text = re.sub(rf'(\[{re.escape(key)}\]\s*=\s*)"[^"]*"', rf'\1"{value}"', text)
    if text != original:
        lang.write_text(text)

# The underlying app is the proven NTAG emulator, but the visible app is now
# NFC Actions. It retains the same mini-app ID to avoid risky launcher changes.
app_amiibo = SRC / "app/amiibo/app_amiibo.c"
text = app_amiibo.read_text()
text = re.sub(r'\.name\s*=\s*"[^"]*"', '.name = "NFC Actions"', text, count=1)
# No Amiibo key-loading is needed for NFC Actions.
text = text.replace('amiibo_helper_try_load_amiibo_keys_from_vfs();', '(void)0;')
app_amiibo.write_text(text)

# Do not load Amiibo keys during normal boot either.
main = SRC / "main.c"
text = main.read_text()
text = text.replace('amiibo_helper_try_load_amiibo_keys_from_vfs();', '(void)0;')
main.write_text(text)

# Built-in NFC Actions. These are the exact URI payloads reconstructed from the
# v8 540-byte presets. Selecting one writes the NDEF URI directly into the
# active NTAG215 emulator. No .bin import or external-storage setup is required.
actions_scene = r'''#include "app_amiibo.h"
#include "amiibo_scene.h"
#include "mini_app_launcher.h"
#include "mini_app_registry.h"
#include "ntag_emu.h"
#include "ntag_store.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ICON_ACTION 0xe1ed
#define ICON_HOME   0xe1f0

typedef struct {
    const char *label;
    const char *uri;
} wuzplay_action_t;

static const wuzplay_action_t actions[] = {
    {"System Status", "scriptable:///run?scriptName=CyberSync&mode=system"},
    {"Dashboard", "scriptable:///run?scriptName=CyberSync&mode=dashboard"},
    {"Network", "scriptable:///run?scriptName=CyberSync&mode=network"},
    {"Cyber Tools", "scriptable:///run?scriptName=CyberSync&mode=tools"},
    {"Alerts", "scriptable:///run?scriptName=CyberSync&mode=alerts"},
    {"Run CyberSync", "scriptable:///run?scriptName=CyberSync&mode=all"},
    {"Open WuzSync", "scriptable:///run?scriptName=WuzSync"},
    {"Govee On", "shortcuts://run-shortcut?name=Govee%20On"},
    {"Govee Off", "shortcuts://run-shortcut?name=Govee%20Off"},
    {"Govee Bright", "shortcuts://run-shortcut?name=Govee%20Bright"},
    {"Govee Relax", "shortcuts://run-shortcut?name=Govee%20Relax"},
    {"Govee Movie", "shortcuts://run-shortcut?name=Govee%20Movie"},
    {"Govee Red", "shortcuts://run-shortcut?name=Govee%20Red"},
    {"Govee Blue", "shortcuts://run-shortcut?name=Govee%20Blue"},
    {"Drive Home", "shortcuts://run-shortcut?name=Drive%20Home"},
    {"Find Car", "shortcuts://run-shortcut?name=Find%20Car"},
    {"Flashlight", "shortcuts://run-shortcut?name=Flashlight"},
    {"Quick Note", "shortcuts://run-shortcut?name=Quick%20Note"},
    {"10m Timer", "shortcuts://run-shortcut?name=10%20Minute%20Timer"},
};

static bool set_uri_tag(app_amiibo_t *app, const char *uri) {
    const size_t uri_len = strlen(uri);
    const size_t payload_len = 1u + uri_len;       /* URI prefix byte + URI */
    const size_t ndef_len = 4u + payload_len;      /* hdr, type-len, payload-len, 'U' + payload */
    if (uri_len > 220u || ndef_len > 254u) return false;

    ntag_store_new_rand(&app->ntag);
    app->ntag.read_only = true;

    /* Preserve manufacturer/config pages; clear only the first user-memory area. */
    memset(&app->ntag.data[16], 0, 256);
    size_t p = 16;
    app->ntag.data[p++] = 0x03;                    /* NDEF Message TLV */
    app->ntag.data[p++] = (uint8_t)ndef_len;
    app->ntag.data[p++] = 0xD1;                    /* MB|ME|SR|TNF well-known */
    app->ntag.data[p++] = 0x01;                    /* type length */
    app->ntag.data[p++] = (uint8_t)payload_len;
    app->ntag.data[p++] = 0x55;                    /* 'U' URI record */
    app->ntag.data[p++] = 0x00;                    /* no URI prefix compression */
    memcpy(&app->ntag.data[p], uri, uri_len);
    p += uri_len;
    app->ntag.data[p++] = 0xFE;                    /* terminator TLV */

    ntag_emu_set_tag(&app->ntag);
    return true;
}

static void on_selected(mui_list_view_event_t event, mui_list_view_t *list, mui_list_item_t *item) {
    if (event != MUI_LIST_VIEW_EVENT_SELECTED) return;
    app_amiibo_t *app = list->user_data;
    if (item->icon == ICON_HOME || item->user_data == NULL) {
        mini_app_launcher_kill(mini_app_launcher(), MINI_APP_ID_AMIIBO);
        return;
    }
    const wuzplay_action_t *action = (const wuzplay_action_t *)item->user_data;
    (void)set_uri_tag(app, action->uri);
    /* The selected tag stays active. Hold the Wuzplay at the top of the iPhone. */
}

void amiibo_scene_file_browser_on_enter(void *user_data) {
    app_amiibo_t *app = user_data;
    mui_list_view_clear_items(app->p_list_view);
    mui_list_view_add_item(app->p_list_view, ICON_HOME, "Back", NULL);
    for (size_t i = 0; i < sizeof(actions) / sizeof(actions[0]); ++i) {
        mui_list_view_add_item(app->p_list_view, ICON_ACTION, actions[i].label, (void *)&actions[i]);
    }
    mui_list_view_set_selected_cb(app->p_list_view, on_selected);
    mui_view_dispatcher_switch_to_view(app->p_view_dispatcher, AMIIBO_VIEW_ID_LIST);
}

void amiibo_scene_file_browser_on_exit(void *user_data) {
    (void)user_data;
}
'''
(SRC / "app/amiibo/scene/amiibo_scene_file_browser.c").write_text(actions_scene)

# Keep all four known games explicitly present. Abort reconstruction rather than
# silently producing a DFU with a missing game.
game_list = (SRC / "app/game/scene/game_scene_game_list.c").read_text()
for marker in ("tiny_arkanoid_run", "tiny_invaders_run", "tiny_lander_run", "tiny_tris_run"):
    if marker not in game_list:
        raise SystemExit(f"required game missing from source: {marker}")

# Required custom capabilities from the full custom snapshot.
checks = {
    SRC / "mod/settings.h": ("return_key", "display_flip"),
    SRC / "mui/mui_input.c": ("INPUT_KEY_BACK",),
    SRC / "app/chameleon/scene/chameleon_scene_menu_card_advanced.c": ("ATS",),
}
for path, needles in checks.items():
    data = path.read_text(errors="ignore")
    for needle in needles:
        if needle not in data:
            raise SystemExit(f"required capability marker {needle!r} missing from {path}")

print("Wuzplay v8 full feature patch applied")
print("Top menu: NFC Actions / NFC Cards / Media Player / Cyber Arcade / Wireless Files / System")
print(f"Built-in NFC actions: {len(actions_scene.split('{\"')[1:]) if False else 19}")
print("Games: Arkanoid / Invaders / Lander / Tris")
