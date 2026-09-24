# BLINK Landscape Desktop

A landscape-first AI pocket desktop for iPhone that preserves the reading canvas while its compact app-owned keyboard is open.

Install URL: https://blink-landscape-desktop.zeusonsora.chatgpt.site

## What works

- Responsive portrait and landscape layouts
- Warm-paper reading canvas inside a minimal black shell
- Compact QWERTY keyboard that does not summon the full Apple keyboard
- Persistent on-device conversations and new canvases
- Secure server-side OpenAI Responses API endpoint with retry and connection states
- Working type, punctuation, backspace, space, send, hide, read, canvas, voice dictation, share, and immersive controls
- Bluetooth/hardware-keyboard input without opening Apple's on-screen keyboard
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

Open the install URL in Safari, tap **Share**, choose **Add to Home Screen**, and launch BLINK from its new icon. Rotate the iPhone to landscape after opening BLINK.

## OpenAI connection

The browser calls `/api/chat`; the Site worker calls OpenAI's Responses API after the server-side `OPENAI_API_KEY` secret is connected. Until then, the app shows **CONNECT AI**. Never place that key in browser code, the PWA bundle, or Git.

## Project notes

The Obsidian handoff is at `docs/obsidian/BLINK Landscape Desktop.md`.

Made by Zeus · BLINK / NODE · 2026
