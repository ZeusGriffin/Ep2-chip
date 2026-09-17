# BLINK Face Motion — tested core

The hardware-neutral face motion engine compiles under C++17 and passed a local behavior test before board integration.

Validated behaviors:
- eye openness remains within 0..1
- mouth openness remains within 0..1
- gaze remains within normalized bounds
- periodic blinking runs without blocking delays
- idle gaze drift works
- listening state recenters attention
- thinking state moves faster/more actively
- speaking state responds to supplied speech level
- reaction state returns to idle after timeout

What this does **not** prove yet:
- exact display rendering
- exact frame rate on the user's board
- touch/button mapping
- audio timing
- compatibility with the actual miner hardware

Those require the exact ESP32/display identification first.

Design intent borrowed from the referenced Desk Buddy video:
- slower idle eye drift
- natural pauses instead of constant motion
- short reaction animations
- expression changes tied to state
- subtle movement that makes the character feel alive without looking like a toy

The BLINK/NODE visual style remains black/white/minimal rather than copying the Desk Buddy appearance.
