#!/usr/bin/env python3
from pathlib import Path
import re
import shutil
import sys

ROOT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path("pixljs-source").resolve()
SRC = ROOT / "fw/application/src"
MAKEFILE = ROOT / "fw/application/Makefile"
if not SRC.exists():
    raise SystemExit(f"pixl.js source tree not found: {SRC}")

# Keep the proven custom base, but do not reuse or modify any Amiibo application
# directory. NFC Actions is its own app under app/nfc_actions.
mini_app_data = r'''#include "mini_app_defines.h"
#include "app_status_bar.h"
#include "app_desktop.h"
#include "app_nfc_actions.h"
#include "app_ble.h"
#include "app_player.h"
#include "app_settings.h"
#include "app_chameleon.h"
#include "app_game.h"
#include <stddef.h>

const mini_app_t* mini_app_registry[] = {
    &app_status_bar_info,
    &app_desktop_info,
    &app_nfc_actions_info,
    &app_chameleon_info,
#ifdef APP_PLAYER_ENABLE
    &app_player_info,
#endif
#ifdef APP_GAME_ENABLE
    &app_game_info,
#endif
    &app_ble_info,
    &app_settings_info
};

const uint32_t mini_app_num = sizeof(mini_app_registry) / sizeof(mini_app_registry[0]);
'''
(SRC / "core/mini_app_data.c").write_text(mini_app_data)

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

# Remove the legacy key-loading call; NFC Actions uses plain URI NTAG records.
main = SRC / "main.c"
text = main.read_text().replace('amiibo_helper_try_load_amiibo_keys_from_vfs();', '(void)0;')
main.write_text(text)

# Dedicated NFC Actions app. MINI_APP_ID_AMIIBO is retained only as the numeric
# legacy slot identifier to avoid a risky enum/layout migration; no Amiibo app
# source is built and no custom code is placed in an Amiibo directory.
nfc_dir = SRC / "app/nfc_actions"
nfc_dir.mkdir(parents=True, exist_ok=True)
(nfc_dir / "app_nfc_actions.h").write_text(r'''#ifndef APP_NFC_ACTIONS_H
#define APP_NFC_ACTIONS_H

#include "mini_app_defines.h"
#include "mui_include.h"
#include "ntag_def.h"

typedef struct {
    mui_view_dispatcher_t *p_view_dispatcher;
    mui_list_view_t *p_list_view;
    ntag_t ntag;
} app_nfc_actions_t;

extern mini_app_t app_nfc_actions_info;

#endif
''')

(nfc_dir / "app_nfc_actions.c").write_text(r'''#include "app_nfc_actions.h"
#include "i18n/language.h"
#include "mini_app_launcher.h"
#include "mini_app_registry.h"
#include "ntag_emu.h"
#include "ntag_store.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

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

static bool set_uri_tag(app_nfc_actions_t *app, const char *uri) {
    const size_t uri_len = strlen(uri);
    const size_t payload_len = 1u + uri_len;
    const size_t ndef_len = 4u + payload_len;
    if (uri_len > 220u || ndef_len > 254u) return false;

    ntag_store_new_rand(&app->ntag);
    app->ntag.read_only = true;
    memset(&app->ntag.data[16], 0, 256);

    size_t p = 16;
    app->ntag.data[p++] = 0x03;
    app->ntag.data[p++] = (uint8_t)ndef_len;
    app->ntag.data[p++] = 0xD1;
    app->ntag.data[p++] = 0x01;
    app->ntag.data[p++] = (uint8_t)payload_len;
    app->ntag.data[p++] = 0x55;
    app->ntag.data[p++] = 0x00;
    memcpy(&app->ntag.data[p], uri, uri_len);
    p += uri_len;
    app->ntag.data[p++] = 0xFE;

    ntag_emu_set_tag(&app->ntag);
    return true;
}

static void nfc_actions_selected(mui_list_view_event_t event, mui_list_view_t *list, mui_list_item_t *item) {
    if (event != MUI_LIST_VIEW_EVENT_SELECTED) return;
    app_nfc_actions_t *app = list->user_data;
    if (item->icon == ICON_BACK || item->user_data == NULL) {
        mini_app_launcher_kill(mini_app_launcher(), MINI_APP_ID_AMIIBO);
        return;
    }
    const wuzplay_action_t *action = (const wuzplay_action_t *)item->user_data;
    (void)set_uri_tag(app, action->uri);
}

static void app_nfc_actions_on_run(mini_app_inst_t *p_app_inst) {
    app_nfc_actions_t *app = mui_mem_malloc(sizeof(app_nfc_actions_t));
    memset(app, 0, sizeof(*app));
    p_app_inst->p_handle = app;

    app->p_view_dispatcher = mui_view_dispatcher_create();
    app->p_list_view = mui_list_view_create();
    mui_list_view_set_user_data(app->p_list_view, app);
    mui_list_view_set_selected_cb(app->p_list_view, nfc_actions_selected);

    mui_list_view_add_item(app->p_list_view, ICON_BACK, "Back", NULL);
    for (size_t i = 0; i < sizeof(actions) / sizeof(actions[0]); ++i) {
        mui_list_view_add_item(app->p_list_view, ICON_FILE, actions[i].label, (void *)&actions[i]);
    }

    mui_view_dispatcher_add_view(app->p_view_dispatcher, 0, mui_list_view_get_view(app->p_list_view));
    mui_view_dispatcher_attach(app->p_view_dispatcher, MUI_LAYER_FULLSCREEN);
    mui_view_dispatcher_switch_to_view(app->p_view_dispatcher, 0);
}

static void app_nfc_actions_on_kill(mini_app_inst_t *p_app_inst) {
    app_nfc_actions_t *app = p_app_inst->p_handle;
    if (!app) return;
    mui_view_dispatcher_detach(app->p_view_dispatcher, MUI_LAYER_FULLSCREEN);
    mui_view_dispatcher_free(app->p_view_dispatcher);
    mui_list_view_free(app->p_list_view);
    mui_mem_free(app);
    p_app_inst->p_handle = NULL;
}

static void app_nfc_actions_on_event(mini_app_inst_t *p_app_inst, mini_app_event_t *p_event) {
    (void)p_app_inst;
    (void)p_event;
}

mini_app_t app_nfc_actions_info = {
    .id = MINI_APP_ID_AMIIBO,
    .name = "NFC Actions",
    .name_i18n_key = _L_APP_AMIIBO,
    .icon = 0xe1ed,
    .deamon = false,
    .sys = false,
    .hibernate_enabled = false,
    .icon_32x32 = &app_card_emulator_32x32,
    .run_cb = app_nfc_actions_on_run,
    .kill_cb = app_nfc_actions_on_kill,
    .on_event_cb = app_nfc_actions_on_event,
};
''')

# Strip all app-level Amiibo/AmiiboDB/AmiiboLink source/include entries.
mk = MAKEFILE.read_text()
mk = "".join(
    line for line in mk.splitlines(True)
    if "$(PROJ_DIR)/app/amiibo" not in line
    and "$(PROJ_DIR)/app/amiidb" not in line
    and "$(PROJ_DIR)/app/amiibolink" not in line
)

# Add the standalone NFC Actions app without fragile Makefile continuation edits.
if "app/nfc_actions/app_nfc_actions.c" not in mk:
    mk += "\nSRC_FILES += $(PROJ_DIR)/app/nfc_actions/app_nfc_actions.c\n"
if "INC_FOLDERS += $(PROJ_DIR)/app/nfc_actions" not in mk:
    mk += "INC_FOLDERS += $(PROJ_DIR)/app/nfc_actions\n"
MAKEFILE.write_text(mk)

# Remove the application directories from the generated source tree so there is
# literally no app/amiibo folder in the firmware build workspace.
for rel in ("app/amiibo", "app/amiidb", "app/amiibolink"):
    p = SRC / rel
    if p.exists():
        shutil.rmtree(p)

# Hard gates.
game_list = (SRC / "app/game/scene/game_scene_game_list.c").read_text()
for marker in ("tiny_arkanoid_run", "tiny_invaders_run", "tiny_lander_run", "tiny_tris_run"):
    if marker not in game_list:
        raise SystemExit(f"required game missing from source: {marker}")
settings_h = (SRC / "mod/settings.h").read_text(errors="ignore")
for marker in ("return_key", "display_flip"):
    if marker not in settings_h:
        raise SystemExit(f"required custom setting missing: {marker}")
if "INPUT_KEY_BACK" not in (SRC / "mui/mui_input.c").read_text(errors="ignore"):
    raise SystemExit("required Back input support missing")
for rel in ("app/amiibo", "app/amiidb", "app/amiibolink"):
    if (SRC / rel).exists():
        raise SystemExit(f"forbidden app folder still exists: {rel}")

print("Wuzplay v8 full feature patch applied")
print("NFC Actions lives only in app/nfc_actions")
print("Amiibo/AmiiboDB/AmiiboLink app folders removed from generated build")
print("Built-in NFC actions: 19")
print("Games: Arkanoid / Invaders / Lander / Tris")
