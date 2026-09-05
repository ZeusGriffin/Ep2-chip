---
description: "Bring Copilot fully up to speed on Cardputer ADV Phone Mirror and continue from the next unpassed test."
---
You are taking over the Cardputer ADV Phone Mirror repository.

Read these files before editing anything:

- `AGENTS.md`
- `.github/copilot-instructions.md`
- `PROJECT_STATUS.md`
- `docs/ARCHITECTURE.md`
- `docs/TEST_PLAN.md`
- `docs/PROTOCOL.md`
- `firmware/src/main.cpp`
- `firmware/platformio.ini`
- `tools/pc_sender.py`
- `ios/CardputerBroadcast/SampleHandler.swift`
- `ios/project.yml`

Then do the following:

1. Summarize the current architecture in no more than 10 bullets.
2. Separate **proven**, **implemented but untested**, and **planned** items.
3. Identify the next unpassed Phase 1 acceptance check.
4. Inspect the code for anything that would block that exact test.
5. Make only the changes required to maximize the chance that the next test passes.
6. If you can compile or lint locally, do so and report exact results. Do not claim hardware validation without real hardware.
7. Update `PROJECT_STATUS.md` with the work performed and the next physical test the user must run.
8. Do not start Steam Link / Phase 2 work yet.

Project ownership/credit: Made by Zeus / 925 Studios.
