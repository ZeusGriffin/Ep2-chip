#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path('pixljs-source').resolve()
makefile = root / 'fw/application/Makefile'
text = makefile.read_text()
anchor = 'CFLAGS += -DBOARD_CUSTOM\n'
flags = (
    'CFLAGS += -DAPP_GAME_ENABLE\n'
    'CFLAGS += -DAPP_PLAYER_ENABLE\n'
    'CFLAGS += -DAPP_LEGLAMIIBO_ENABLE\n'
)
if '-DAPP_GAME_ENABLE' not in text:
    if anchor not in text:
        raise SystemExit('BOARD_CUSTOM CFLAGS anchor not found')
    text = text.replace(anchor, anchor + flags, 1)
makefile.write_text(text)
print('Forced release apps: Cyber Arcade / Media Player / NFC Actions / Wireless Files')
