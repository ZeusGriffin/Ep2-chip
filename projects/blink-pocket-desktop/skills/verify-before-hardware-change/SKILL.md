---
name: verify-before-hardware-change
description: Use before writing or flashing ANY pin/GPIO/polarity/wiring change to a physical device. Find an authoritative source first; never trial-and-error guess on hardware.
---

# Verify Before Hardware Change

## Why this exists
On BLINK (NM-TV-154), two flash attempts used community-sourced pins that
turned out correct, but a guessed GPIO21 polarity (HIGH instead of the
documented LOW) burned a full round-trip and a physical confirm before the
real vendor-adjacent source (RockBase-iot/NM-TV-154 HAL docs) was checked.
The fix took one line once the source was found. Earlier, an unauthenticated
GitHub Code Search API call hit a rate limit and got treated as a dead end
instead of switched to `git clone --depth 1` + local grep, which works
without auth and has no rate limit.

## Trigger
Any pin, GPIO, polarity, register, wiring, or protocol change to a physical
device — before the FIRST attempt, and before every RE-attempt after a
failure. Applies even when a value "looks standard" or was seen in a blog
post/forum.

## Workflow
1. Identify the exact board/chip/part identifier (not a lookalike).
2. Search for an authoritative or independently-verified reference for that
   exact part: vendor docs/schematic first, then an independent open-source
   HAL/bring-up repo for the exact board (not just the app that ships on it —
   distribution repos are often binary-only).
3. If a hosted API (e.g. GitHub search) is rate-limited or blocked, do not
   stop — `git clone --depth 1 <repo>` and grep the source directly. No auth,
   no rate limit.
4. Cross-check against a second independent source when one exists.
5. Make the smallest possible code change that matches the cited source.
   Comment the source next to the value.
6. Flash/test ONCE. Get a real device confirmation (visual, serial, etc.)
   before considering a second change.
7. If it fails: go back to step 2 (more source), not step 5 (another guess).
   Never iterate through untraced values on real hardware.

## Reusable prompt
"Before touching this hardware pin/config, find the authoritative source
(vendor docs, schematic, or an independent working repo for this exact
board/part) and cite it. Do not guess. If a search API is rate-limited,
clone the repo and grep it instead of stopping. One verified fix, one
flash, one confirmation — no trial-and-error loops on the device."

