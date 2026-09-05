# AGENTS.md — Cardputer ADV Phone Mirror

This file is the operational brief for coding agents.

@.github/copilot-instructions.md
@PROJECT_STATUS.md
@docs/ARCHITECTURE.md
@docs/TEST_PLAN.md
@docs/PROTOCOL.md

## Agent workflow

When given a task:

1. Identify whether it belongs to firmware, iOS, tooling, docs, or Phase 2.
2. Reject accidental Phase-2 scope creep until Phase 1 is validated.
3. Inspect current code before generating replacements.
4. Prefer patches over rewrites.
5. If changing firmware, keep serial output sufficient to diagnose boot, Wi-Fi, TCP, JPEG decode, FPS, and heap.
6. If changing iOS sender behavior, preserve frame-dropping/backpressure behavior; never build an unbounded queue.
7. If changing transport, keep backwards compatibility when practical and document the migration.
8. Add a test or a reproducible manual test step for each bug fix.
9. Update `PROJECT_STATUS.md` with what changed, what was tested, and what remains.

## Never assume

- Never assume the current build has been flashed successfully.
- Never assume the iPhone extension has been signed successfully.
- Never assume Steam Link can be run directly on the ESP32-S3.
- Never assume ReplayKit captures DRM-protected content.
- Never call the project finished because code compiles.
