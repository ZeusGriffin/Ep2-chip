---
project: BLINK Landscape Desktop
status: prototype
platform: iPhone 15 Pro
delivery: PWA
tags: [blink, node, iphone, pwa, product-design]
---

# BLINK Landscape Desktop

## Goal

Make the iPhone usable like a small landscape computer without letting the keyboard cover the reading area.

## Locked visual baseline

- Near-black full-screen shell.
- Warm-paper reading canvas.
- Minimal top controls and small gradient workspace indicators.
- Floating rounded dock inspired by the supplied Cosmos screenshots.
- Wide landscape layout with a workspace rail and large reading pane.

## Core interaction

1. Open the app from the iPhone Home Screen.
2. Rotate the phone to landscape.
3. Read in the large canvas.
4. Tap **Type** to open the compact app-owned keyboard.
5. Tap **Hide** or **Read** to return to the full reading view.

## Current build

- Responsive landscape/portrait interface.
- Compact keyboard that avoids triggering Apple's full-height keyboard.
- Working Canvas, Read, Type, Voice, Share, immersive, and send controls.
- Installable web-app manifest and offline shell.
- Local sample conversation; OpenAI service connection is intentionally not included in this prototype.

## Verification target

- iPhone 15 Pro landscape viewport.
- No horizontal clipping.
- Reading canvas remains visible with the compact keyboard open.
- Touch controls remain usable and text stays readable.

## Next integration

Add a secure server-side OpenAI connection after the interface is approved. Never put an API key in browser code or commit it to Git.
