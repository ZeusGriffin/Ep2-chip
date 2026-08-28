# Touch Hologram — Interactive Hand-Tracked Hologram

This folder preserves a working open-source reference for the **touch / interactive hologram** concept from the YouTube Short:

- Short: https://youtube.com/shorts/LpRHxiUFiiY?is=lEYKqjR9ymEcHfi5
- Upstream implementation: https://github.com/y2xs/Hologram
- Upstream license: MIT

## What this version does

The browser uses the camera to detect one hand with **MediaPipe Hand Landmarker** and renders a particle hologram with **Three.js**.

- Move your hand left/right or up/down → rotate the hologram.
- Change the distance between thumb and index finger → zoom / expand the hologram.
- Black background + additive particles make it suitable for a reflective hologram pyramid, glass/film setup, or a normal screen demo.

## Important distinction

This is **touch-like gesture interaction**, not physical haptic feedback. Your hand controls the image without touching a screen. A true touchable hologram that you can physically feel would require additional haptic hardware such as a focused-ultrasound array or another mid-air feedback system.

## Run it

Because camera access is restricted in modern browsers, serve the folder from localhost instead of double-clicking the HTML file.

```bash
cd external/touch-hologram
python3 -m http.server 8000
```

Then open:

```text
http://localhost:8000
```

Allow camera access when prompted.

## Minimum setup

- Laptop, desktop, Raspberry Pi-class computer, or another device with a modern browser
- Camera / webcam
- Display

Optional for a more convincing physical presentation:

- Clear acrylic hologram pyramid or angled reflective acrylic/glass
- Bright high-contrast display
- Enclosure to reduce ambient light

## Future Zeus build path

1. Keep this MediaPipe version as the software baseline.
2. Add selectable hologram objects and menus.
3. Add pinch, grab, swipe, and tap-zone gestures.
4. Add a dedicated camera or depth sensor for more reliable tracking.
5. Add a physical hologram enclosure / reflective display.
6. If true tactile feedback is wanted later, evaluate an ultrasonic haptic array as a separate hardware module.

## Attribution

The included `index.html` is based on **y2xs/Hologram**, licensed under MIT. The upstream copyright and license are preserved in `LICENSE-UPSTREAM`.
