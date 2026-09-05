---
applyTo: "ios/**"
---
# iOS-specific instructions

The iOS side uses ReplayKit Broadcast Upload Extension to obtain screen frames outside the host app.

- Use Network.framework for the TCP sender unless there is a strong reason to change it.
- Resize/crop before JPEG encoding so the ESP32 receives near-native 240x135 frames.
- Drop frames under backpressure; do not queue frames without a bound.
- Keep the extension memory-conscious and avoid unnecessary image copies.
- Preserve orientation handling using ReplayKit sample metadata.
- Keep ordinary app UI minimal: connection instructions, broadcast picker, and clear status.
- Document that DRM/secure content may be blacked out by iOS.
- Do not add remote-control input to iOS until Phase 1 video validation passes.
