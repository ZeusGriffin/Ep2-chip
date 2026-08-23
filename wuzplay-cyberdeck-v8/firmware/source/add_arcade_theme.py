#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path('pixljs-source').resolve()
SRC = ROOT / 'fw/application/src'
if not SRC.exists():
    raise SystemExit(f'pixl.js source tree not found: {SRC}')

# Compact monochrome tech typography: use the already bundled small pixel font
# in list-based UI. This is an original dot/pixel-tech treatment inspired by the
# clean Nothing aesthetic, not a copy of a proprietary Nothing font.
list_view = SRC / 'mui/view/mui_list_view.c'
text = list_view.read_text()
text = text.replace('#define LIST_ITEM_HEIGHT 13', '#define LIST_ITEM_HEIGHT 11')
text = text.replace('u8g2_font_wqy12_t_gb2312a', 'u8g2_font_likeminecraft_te')
text = text.replace('actual_y + 10, string_get_cstr(item->text)', 'actual_y + 8, string_get_cstr(item->text)')
text = text.replace('actual_y + 10,\n                                              string_get_cstr(item->sub_text)', 'actual_y + 8,\n                                              string_get_cstr(item->sub_text)')
text = text.replace('actual_y + 10,\n                                         string_get_cstr(item->sub_text)', 'actual_y + 8,\n                                         string_get_cstr(item->sub_text)')
text = text.replace('uint32_t focus_h = 12;', 'uint32_t focus_h = 10;')
list_view.write_text(text)

# Two additional native games requested for the all-in-one build. They use the
# same tiny-game display/input driver as Arkanoid/Invaders/Lander/Tris, so they
# live entirely in firmware and need no external files.
game_dir = SRC / 'app/game/port/cyber_games'
game_dir.mkdir(parents=True, exist_ok=True)
(game_dir / 'cyber_games.h').write_text(r'''#pragma once
int cyber_hoops_2k_run(void);
int cyber_fighter_run(void);
''')

(game_dir / 'cyber_games.c').write_text(r'''#include "../common/driver.h"
#include "game_view.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static uint8_t fb[8][128];

static void clear_fb(void) { memset(fb, 0, sizeof(fb)); }
static void px(int x, int y, bool on) {
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;
    uint8_t mask = (uint8_t)(1u << (y & 7));
    if (on) fb[y >> 3][x] |= mask;
    else fb[y >> 3][x] &= (uint8_t)~mask;
}
static void hline(int x0, int x1, int y) {
    if (x0 > x1) { int t=x0; x0=x1; x1=t; }
    for (int x=x0; x<=x1; ++x) px(x,y,true);
}
static void vline(int x, int y0, int y1) {
    if (y0 > y1) { int t=y0; y0=y1; y1=t; }
    for (int y=y0; y<=y1; ++y) px(x,y,true);
}
static void rect(int x, int y, int w, int h) {
    hline(x, x+w-1, y); hline(x, x+w-1, y+h-1);
    vline(x, y, y+h-1); vline(x+w-1, y, y+h-1);
}
static void fill_rect(int x, int y, int w, int h) {
    for (int yy=y; yy<y+h; ++yy) for (int xx=x; xx<x+w; ++xx) px(xx,yy,true);
}
static void flush_fb(void) {
    for (uint8_t page=0; page<8; ++page) {
        JOY_OLED_data_start(page);
        for (uint8_t x=0; x<128; ++x) JOY_OLED_send(fb[page][x]);
        JOY_OLED_end();
    }
}
static bool exit_pressed(void) {
    return game_view_key_pressed(INPUT_KEY_BACK) || JOY_exit();
}
static void draw_score_pips(uint8_t score, int x, int y) {
    score %= 10;
    for (uint8_t i=0; i<score; ++i) fill_rect(x + i*4, y, 2, 3);
}

int cyber_hoops_2k_run(void) {
    JOY_init();
    int player_x = 18;
    int ball_x = player_x + 7, ball_y = 44;
    int vx = 0, vy = 0;
    bool flying = false;
    bool prev_fire = false;
    uint8_t score = 0;

    while (!exit_pressed()) {
        if (!flying) {
            if (JOY_left_pressed() && player_x > 4) player_x--;
            if (JOY_right_pressed() && player_x < 86) player_x++;
            ball_x = player_x + 7; ball_y = 44;
        }
        bool fire = JOY_act_pressed();
        if (fire && !prev_fire && !flying) {
            flying = true; vx = 3; vy = -4;
        }
        prev_fire = fire;

        if (flying) {
            ball_x += vx;
            ball_y += vy;
            if (vy < 5) vy++;
            if (ball_x >= 108 && ball_x <= 116 && ball_y >= 23 && ball_y <= 30 && vy > 0) {
                score++;
                flying = false;
            } else if (ball_x < 0 || ball_x > 127 || ball_y > 62) {
                flying = false;
            }
        }

        clear_fb();
        hline(0,127,58);                  /* floor */
        vline(119,14,38);                 /* backboard */
        hline(108,119,27);                /* rim */
        rect(112,28,6,7);                 /* net */
        rect(player_x,43,8,15);           /* player body */
        fill_rect(player_x+2,39,4,4);     /* head */
        fill_rect(ball_x-1,ball_y-1,3,3); /* ball */
        draw_score_pips(score,3,3);
        flush_fb();
        JOY_idle();
        JOY_DLY_ms(35);
    }
    return 0;
}

static void fighter_sprite(int x, int y, bool punch) {
    fill_rect(x+3,y,4,4);                 /* head */
    rect(x+2,y+5,6,11);                   /* torso */
    vline(x+3,y+16,y+24); vline(x+7,y+16,y+24);
    if (punch) hline(x+8,x+15,y+8);
    else { hline(x-2,x+2,y+8); hline(x+8,x+11,y+8); }
}
static void hp_bar(int x, int y, uint8_t hp, bool right) {
    rect(x,y,43,6);
    int fill = (int)hp * 39 / 10;
    if (fill < 0) fill=0; if (fill > 39) fill=39;
    if (right) fill_rect(x+2 + (39-fill), y+2, fill, 2);
    else fill_rect(x+2,y+2,fill,2);
}

int cyber_fighter_run(void) {
    JOY_init();
    int px = 18, ax = 94;
    uint8_t php = 10, ahp = 10;
    uint8_t ai_timer = 0;
    bool prev_fire = false;
    uint8_t punch_frames = 0;

    while (!exit_pressed()) {
        if (JOY_left_pressed() && px > 2) px--;
        if (JOY_right_pressed() && px < ax-14) px++;
        bool fire = JOY_act_pressed();
        if (fire && !prev_fire) {
            punch_frames = 4;
            if (ax - px < 24 && ahp > 0) ahp--;
        }
        prev_fire = fire;

        if (ahp && php) {
            if (ax - px > 22 && (ai_timer & 1u)) ax--;
            if (++ai_timer >= 18) {
                ai_timer = 0;
                if (ax - px < 25 && php > 0) php--;
            }
        }

        clear_fb();
        hp_bar(2,2,php,false);
        hp_bar(83,2,ahp,true);
        hline(0,127,58);
        fighter_sprite(px,30,punch_frames>0);
        fighter_sprite(ax,30,false);
        if (punch_frames) punch_frames--;
        if (!php || !ahp) {
            /* center marker: left = player win, right = CPU win */
            if (ahp == 0) fill_rect(55,18,7,7); else fill_rect(67,18,7,7);
        }
        flush_fb();
        JOY_idle();
        JOY_DLY_ms(45);

        if (!php || !ahp) {
            JOY_DLY_ms(700);
            php=10; ahp=10; px=18; ax=94; ai_timer=0;
        }
    }
    return 0;
}
''')

# Add the new source to the firmware build.
makefile = ROOT / 'fw/application/Makefile'
mk = makefile.read_text()
needle = '  $(PROJ_DIR)/app/game/port/common/driver.c \\\n'
addition = needle + '  $(PROJ_DIR)/app/game/port/cyber_games/cyber_games.c \\\n'
if 'cyber_games/cyber_games.c' not in mk:
    if needle not in mk:
        raise SystemExit('game driver anchor not found in Makefile')
    mk = mk.replace(needle, addition, 1)
makefile.write_text(mk)

# Add both games to Cyber Arcade while preserving all four verified historical games.
game_list = SRC / 'app/game/scene/game_scene_game_list.c'
gl = game_list.read_text()
if '#include "cyber_games.h"' not in gl:
    gl = gl.replace('#include "tiny_tris.h"', '#include "tiny_tris.h"\n#include "cyber_games.h"')
anchor = '    mui_list_view_add_item(app->p_list_view, ICON_FILE, _T(APP_GAME_TINY_TRIS), tiny_tris_run);'
if 'cyber_hoops_2k_run' not in gl:
    if anchor not in gl:
        raise SystemExit('game menu anchor not found')
    gl = gl.replace(anchor, anchor + '\n    mui_list_view_add_item(app->p_list_view, ICON_FILE, "Cyber Hoops 2K", cyber_hoops_2k_run);\n    mui_list_view_add_item(app->p_list_view, ICON_FILE, "Cyber Fighter", cyber_fighter_run);')
game_list.write_text(gl)

# Ensure the header is visible from the existing game include search path by
# adding the directory next to the other game port include directories.
# The project Makefile derives include dirs explicitly, so patch near tiny_tris.
mk = makefile.read_text()
inc_anchor = '  $(PROJ_DIR)/app/game/port/tiny_tris \\\n'
inc_add = inc_anchor + '  $(PROJ_DIR)/app/game/port/cyber_games \\\n'
if '$(PROJ_DIR)/app/game/port/cyber_games \\' not in mk:
    if inc_anchor not in mk:
        # Some revisions use -I form instead; fall back to appending the include flag.
        mk += '\nINC_FOLDERS += $(PROJ_DIR)/app/game/port/cyber_games\n'
    else:
        mk = mk.replace(inc_anchor, inc_add, 1)
makefile.write_text(mk)

print('Compact tech UI applied')
print('Cyber Arcade: Arkanoid / Invaders / Lander / Tris / Cyber Hoops 2K / Cyber Fighter')
