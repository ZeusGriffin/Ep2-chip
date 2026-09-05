# Copilot — Start Here

This repository now contains a complete Copilot context pack so GitHub Copilot can stay on the same page without you re-explaining the project every time.

## Files Copilot should automatically use

- `.github/copilot-instructions.md` — repository-wide always-on context.
- `AGENTS.md` — coding-agent operating brief.
- `.github/instructions/firmware.instructions.md` — firmware-specific rules.
- `.github/instructions/ios.instructions.md` — iOS-specific rules.
- `.github/instructions/tools.instructions.md` — PC sender/tooling rules.

## Reusable Copilot prompt

Use `.github/prompts/bootstrap-cardputer-mirror.prompt.md` when your Copilot environment supports prompt files.

If you are in a Copilot surface where prompt files are not directly selectable, paste this into chat:

> Read AGENTS.md, .github/copilot-instructions.md, PROJECT_STATUS.md, docs/ARCHITECTURE.md, docs/TEST_PLAN.md, docs/PROTOCOL.md, firmware/src/main.cpp, tools/pc_sender.py, ios/CardputerBroadcast/SampleHandler.swift, and ios/project.yml. Summarize what is proven, what is implemented but untested, and what is planned. Then identify the next unpassed Phase 1 test and make only the changes needed to maximize the chance it passes. Do not start Steam Link Phase 2 until Phase 1 is hardware-validated. Update PROJECT_STATUS.md after meaningful work.

## First job to give Copilot

Ask:

> Prepare this repository for the first real Cardputer ADV hardware test. Verify the PlatformIO configuration and receiver code, run any compile/static checks available, inspect pc_sender.py for protocol compatibility, and update PROJECT_STATUS.md with exact test instructions. Do not claim the hardware works until I report the physical test result.
