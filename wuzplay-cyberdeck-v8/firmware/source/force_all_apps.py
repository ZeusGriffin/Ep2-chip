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
)
if '-DAPP_GAME_ENABLE' not in text or '-DAPP_PLAYER_ENABLE' not in text:
    if anchor not in text:
        raise SystemExit('BOARD_CUSTOM CFLAGS anchor not found')
    text = text.replace(anchor, anchor + flags, 1)
# NFC Actions and Wireless Files are registered directly and do not depend on
# the legacy APP_LEGLAMIIBO_ENABLE gate.
text = text.replace('CFLAGS += -DAPP_LEGLAMIIBO_ENABLE\n', '')
makefile.write_text(text)
print('Forced release apps: Cyber Arcade / Media Player; NFC Actions and Wireless Files are always linked')
