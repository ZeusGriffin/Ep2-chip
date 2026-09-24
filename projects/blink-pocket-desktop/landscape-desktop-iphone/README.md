# BLINK Landscape Desktop

A landscape-first pocket desktop for iPhone that preserves the reading canvas while its compact app-owned keyboard is open.

## What works

- Responsive portrait and landscape layouts
- Warm-paper reading canvas inside a minimal black shell
- Compact QWERTY keyboard that does not summon the full Apple keyboard
- Working type, backspace, space, send, hide, read, canvas, voice-state, share, and immersive controls
- Home Screen web-app manifest and offline shell
- iPhone and Pixel prototype frames for review

## Run locally

```bash
npm ci
npm run dev
```

## Verify

```bash
npm run check:runtime
npm run build
npm run test:sites
```

## iPhone installation

Host the built app over HTTPS, open it in Safari, tap **Share**, and choose **Add to Home Screen**. Rotate the iPhone to landscape after opening BLINK.

## OpenAI connection

This build intentionally uses a local sample conversation. Add the OpenAI connection through a server-side endpoint. Never place an API key in browser code, the PWA bundle, or Git.

## Project notes

The Obsidian handoff is at `docs/obsidian/BLINK Landscape Desktop.md`.

Made by Zeus · BLINK / NODE · 2026
