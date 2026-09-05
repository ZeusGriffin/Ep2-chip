---
applyTo: "tools/**"
---
# Tooling-specific instructions

`pc_sender.py` is a diagnostic harness, not production architecture.

- Keep it easy to run on Windows/macOS with minimal Python dependencies.
- It should make receiver failures obvious: connection errors, frame encode errors, and send rate.
- Preserve the same CMIR framing used by the iOS sender.
- Default to 240x135 and conservative FPS.
- Prefer CLI flags for host, port, FPS, quality, and capture source when adding configurability.
