# Pocket Tank — HTML Edition

A lightweight browser adaptation inspired by [mediacutlet/pocket-tank](https://github.com/mediacutlet/pocket-tank).

## Open it
Open `index.html` in any modern browser. No build step is required.

## Included
- Animated virtual aquarium
- Fish hunger, energy, trust, and goal states
- Tap/click fish for stats
- Tap water or use FEED to drop food
- Light/day-night toggle
- Sand-dollar style reward counter
- Built-in **Check Update** button for the upstream Pocket Tank repository
- Link to the original browser firmware installer

## Easy update
The HTML file checks the latest upstream `main` commit from GitHub when you press **CHECK UPDATE**. The web version is intentionally isolated from upstream firmware so future changes can be merged deliberately without breaking the browser build.

## Important
This is a browser simulation, not a port of the original 14M-parameter embedded LLM. The original project's model/firmware pipeline remains in the upstream repository.

Original project: mediacutlet/pocket-tank  
Original license: MIT  
HTML adaptation location: ZeusGriffin/Ep2-chip/projects/pocket-tank-html
