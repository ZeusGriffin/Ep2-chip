---
project: BLINK Landscape Desktop
status: deployed-ai-authorization-pending
platform: iPhone 15 Pro
delivery: PWA
site: https://blink-landscape-desktop.zeusonsora.chatgpt.site
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
- Persistent conversations stored on the device, with reusable canvases.
- Working Canvas, Read, Type, Voice dictation, Share, immersive, and send controls.
- Hardware-keyboard input that does not summon the Apple keyboard.
- Installable web-app manifest, app icon, and offline shell.
- Secure `/api/chat` worker route using OpenAI's Responses API.
- Loading, retry, disconnected, and ready connection states.

## Verification target

- iPhone 15 Pro landscape viewport.
- No horizontal clipping.
- Reading canvas remains visible with the compact keyboard open.
- Touch controls remain usable and text stays readable.

## Secure AI connection

The OpenAI endpoint is deployed, but the production Site still needs its `OPENAI_API_KEY` server-side secret. OpenAI Developers is installed; OpenAI Platform authorization must finish before the key can be created and attached. The key never enters browser code, local storage, this Obsidian note, or Git.

## Install URL

https://blink-landscape-desktop.zeusonsora.chatgpt.site
