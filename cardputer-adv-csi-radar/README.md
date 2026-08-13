# Cardputer ADV Wi-Fi CSI Human-Detection Radar Build

Project owner: Zeston Griffin / Made by Zeus  
Repository: `ZeusGriffin/Ep2-chip`  
Prepared: 2026-08-13

## Source videos

- Main build: [$50 Human Detecting Radar Feels Illegal (It's Easy)](https://www.youtube.com/watch?v=HqTxv7y6sDM)
- Additional visual/build reference: [YouTube Short UnLvBNIWIpo](https://www.youtube.com/shorts/UnLvBNIWIpo)

## What the main build actually uses

The detection system is **Wi-Fi CSI radar**, not a separate mmWave sensor. The Cardputer ADV analyzes changes in 2.4 GHz Wi-Fi reflections to detect motion/presence. The current supported firmware is **WiFi CSI Radar V1.5 by Zeloksa**.

Important: the radar's source code is currently private/proprietary. The developer distributes a compiled firmware through **M5Burner**. Do not download a random `.bin` or clone a look-alike repository and assume it is the correct firmware.

Official project and instructions:

- Firmware information/manual: https://github.com/Zeloksa/Cardputer-ADV-CSI-Radar
- M5Burner downloads: https://docs.m5stack.com/en/download
- M5Burner tutorial: https://docs.m5stack.com/en/uiflow/m5burner/intro
- Cardputer ADV documentation: https://docs.m5stack.com/en/core/Cardputer-Adv
- Official Cardputer ADV product: https://shop.m5stack.com/products/m5stack-cardputer-adv-version-esp32-s3

## Buy list

### Required for the radar feature

| Qty | Item | Exact requirement | Fast option | Cheaper option |
|---:|---|---|---|---|
| 1 | M5Stack Cardputer ADV | ADV model with Stamp-S3A/ESP32-S3, not an unrelated ESP32 keyboard | [Amazon official listing](https://www.amazon.com/M5stack-Official-Cardputer-Version-ESP32-S3/dp/B0G6ZN35ZX) | [M5Stack official store](https://shop.m5stack.com/products/m5stack-cardputer-adv-version-esp32-s3) or search the official M5Stack AliExpress store |
| 1 | USB-C data cable | Must carry data, not charge-only | Amazon/local electronics store | Any known data-capable USB-C cable |
| 1 | Windows, macOS, or Linux computer | Needed to run M5Burner | Already owned | Already owned |
| 1 | 2.4 GHz Wi-Fi network | Radar depends on changing Wi-Fi reflections | Existing router/hotspot | Existing router/hotspot |

No external mmWave sensor, LoRa module, GPS module, or microSD card is required for the basic CSI radar.

### Required only for your large external display/clamshell version

Buy these only if you do not already own them. The stock CSI V1.5 binary is designed for the Cardputer's built-in screen; it does **not** document support for your ILI9341 display. These parts prepare the physical dual-screen build, but custom/open firmware work is required to put the radar UI on the large screen.

| Qty | Item | Exact requirement | Fast option | Cheaper option |
|---:|---|---|---|---|
| 1 | 2.8-inch ILI9341 TFT | 320×240, 4-wire SPI, PCB module, 3.3 V logic; non-touch is simplest | [Amazon non-touch search](https://www.amazon.com/s?k=2.8%22+ili9341+spi+tft+lcd+display+non+touch) or [known SPI module](https://www.amazon.com/2-8-inch-SPI-module-ILI9341/dp/B0C7L1SY7V) | [AliExpress search](https://www.aliexpress.com/wholesale?SearchText=2.8+inch+ILI9341+SPI+320x240) |
| 1 | 2.54 mm connector kit | Female header/housing and crimp contacts for a removable harness | [Amazon search](https://www.amazon.com/s?k=2.54mm+dupont+connector+kit+crimp) | [AliExpress search](https://www.aliexpress.com/wholesale?SearchText=2.54mm+dupont+connector+kit+crimp) |
| 1 | 28 AWG stranded wire kit | Flexible multicolor wire; seven conductors minimum | [Amazon search](https://www.amazon.com/s?k=28+awg+stranded+hookup+wire+kit) | [AliExpress search](https://www.aliexpress.com/wholesale?SearchText=28awg+stranded+wire+kit) |
| 1 | Heat-shrink assortment | Insulate solder joints and strain-relieve harness | [Amazon search](https://www.amazon.com/s?k=heat+shrink+tubing+assortment) | [AliExpress search](https://www.aliexpress.com/wholesale?SearchText=heat+shrink+tubing+assortment) |
| 1 | Fine-tip soldering setup | Iron, electronics solder, flux | Local/Amazon | Existing tools |
| 1 | Hinged enclosure | Must fit Cardputer ADV plus approximately 50×86 mm display PCB; verify your exact module first | 3D print/custom case | 3D print/custom case |

### Optional but useful

- USB-C extension or right-angle adapter for clean access inside the enclosure.
- Small inline connector so the lid/display can be removed.
- Multimeter for continuity and voltage checks.
- M2/M2.5 screws, brass inserts, foam tape, and strain relief.
- A second Cardputer ADV if you want to keep the radar installed while preserving your everyday cyberdeck firmware.

## Existing external-display wiring reference

Your archived dual-screen pin plan is:

| ILI9341 pin | Cardputer ADV GPIO |
|---|---:|
| CS | GPIO 5 |
| RST | GPIO 3 |
| DC / RS | GPIO 6 |
| MOSI / SDI | GPIO 14 |
| SCK / CLK | GPIO 40 |
| MISO / SDO | Not connected |
| VCC | 3.3 V |
| GND | GND |

Rules:

1. Use 3.3 V logic.
2. Connect ground before testing signals.
3. Do not connect MISO unless the firmware explicitly uses it.
4. Keep GPS disabled in this configuration because the archived project notes identify GPIO overlap.
5. Confirm the Cardputer ADV expansion-pin map and the exact screen PCB labels before soldering.
6. Power off the Cardputer before changing wiring.

## Correct firmware download/install location

The correct radar firmware is not downloaded from this repository.

1. Download **M5Burner** from https://docs.m5stack.com/en/download.
2. Install it on Windows, macOS, or Linux.
3. Connect the Cardputer ADV with a USB-C **data** cable.
4. If the device is not detected, place it in download mode: power it off, hold the G0 button, power it on, then release G0.
5. In M5Burner, select the Cardputer/Cardputer ADV category.
6. Search for **WiFi CSI Radar** or **Zeloksa**.
7. Select **V1.5** (or a later official version only after reviewing its release notes).
8. Select the correct serial/COM port and burn the firmware.
9. Do not flash the Wuzplay Nordic DFU ZIP; that belongs to a different device and repository.
10. After a successful burn, restart the Cardputer.

## First-use instructions

### 1D motion/presence mode

1. Put the Cardputer on a stable, flat surface. Do not hold it during calibration.
2. Press Enter.
3. During the 12-second calibration, leave the space or remain completely still.
4. When calibration completes, movement changes the CSI baseline and triggers the radar.
5. Recalibrate after moving furniture, opening/closing major barriers, moving the Cardputer, or materially changing the room.

### 2D spatial mode — active beta

1. Press `R` to select 2D mode.
2. Press `D` to enable the local web UI and display its IP address.
3. From a phone/computer on the same network, open that IP in a browser.
4. Select **TRAIN EMPTY** and leave the room for seven seconds.
5. Train all four corners: TL, TR, BL, and BR.
6. At each corner, record the four facing directions and the rotation/center profile.
7. After all 20 position profiles plus the empty profile are trained, test the tracking dot.
8. Treat the location result as experimental, not safety-critical.

### Room mapping

1. Enter Mapping Mode.
2. Hold the Cardputer flat against your chest with the screen facing outward.
3. Start mapping and walk at a steady pace, landing each step clearly on the heel.
4. Stop at corners and turn approximately 90 degrees.
5. Close the loop, then stop the mapping run.
6. Verify the generated shape and dimensions; repeat if missed steps distort the map.

## Controls for WiFi CSI Radar V1.5

| Key | Function |
|---|---|
| Enter | Toggle/start radar; start calibration; hold to reset map |
| I | Three-page interactive manual |
| D | Web server and IP overlay |
| R | Switch 1D/2D |
| H | MAX/AUTO processing mode |
| , / . | Manual frequency adjustment |
| - / = | Volume |
| [ / ] | Screen brightness |
| Esc / backtick | Back/exit |
| Delete | Factory reset stored Wi-Fi configuration |

## What can and cannot be combined today

### Ready now

- Cardputer ADV running WiFi CSI Radar on its built-in 1.14-inch screen.
- 1D presence/motion detection.
- Beta 2D profiling through the local web UI.
- Room mapping using the Cardputer ADV IMU.
- Physical large-screen clamshell and harness preparation.

### Not yet proven

- The proprietary CSI Radar V1.5 binary drawing directly to the external ILI9341.
- Merging the proprietary radar engine into our custom dual-screen firmware.
- Running CSI Radar simultaneously with other custom cyberdeck features in one firmware image.

The safe route is to flash and validate the official radar on the built-in screen first. Then use a second, open-source implementation or obtain source/API support from the radar developer before attempting large-screen integration.

## Accuracy, privacy, and safe-use notes

- Wi-Fi CSI indicates disturbances in radio reflections; it is not a camera and cannot identify a person.
- Motion through walls/doors depends heavily on construction, router placement, channel conditions, and calibration.
- False positives can come from pets, fans, doors, furniture movement, neighboring activity, or RF changes.
- Do not use this as a life-safety alarm, medical monitor, or sole security system.
- Use only in spaces and on networks where you have permission. Do not use it to secretly monitor people.
- MAX mode increases heat and battery drain.

## Validation checklist

- [ ] Exact device is Cardputer ADV.
- [ ] USB cable supports data.
- [ ] M5Burner recognizes the correct serial port.
- [ ] Firmware search result is published as WiFi CSI Radar by Zeloksa.
- [ ] Official radar works on the built-in screen before enclosure changes.
- [ ] 1D calibration completes with the device stationary.
- [ ] Web UI opens from the same network.
- [ ] External TFT module is confirmed as ILI9341 SPI 320×240.
- [ ] Display wiring is continuity-tested with power off.
- [ ] 3.3 V and ground are verified before signal wires.
- [ ] GPS remains disabled while the archived overlapping pin plan is used.
- [ ] Screen rotation and X/Y direction are tested before final enclosure assembly.

## Screen orientation test plan

Because the previous display build was flipped/reversed, do not permanently mount the screen until all four checks pass:

1. Draw labels at top, bottom, left, and right.
2. Draw a red pixel/block at logical coordinate (0,0).
3. Confirm portrait/landscape rotation.
4. Confirm X and Y are not mirrored.
5. Adjust the display rotation/MADCTL setting in the custom open firmware.
6. Re-run the test before making a DFU/release package.

## Status of the second Short

The URL is preserved above as a project reference. Its public metadata was not reliably retrievable during this update, so no unverified hardware or wiring claims from that Short have been added. Before buying a unique part shown only in that Short, capture its title/description or a clear screenshot and verify its model number.


## Top-screen shortlist: selected screen plus four alternatives

The top screen should use a true **SPI breakout module**, not an Arduino UNO-style parallel shield. A non-touch module is preferred because the Cardputer keyboard already supplies input and omitting touch saves pins, wiring, thickness, and power.

| Version | Screen | Resolution / bus | Best use | Software change | Physical tradeoff | Buy |
|---:|---|---|---|---|---|---|
| 1 — Selected | **2.8-inch ILI9341 TFT** | 320×240 SPI | Best overall clamshell balance | Lowest; matches the archived driver and wiring plan | Medium lid; common boards are about 50×86 mm | [Amazon non-touch](https://www.amazon.com/2-8-inch-SPI-module-ILI9341/dp/B0C7L1SY7V) · [Amazon alternatives](https://www.amazon.com/s?k=2.8%22+ILI9341+SPI+non+touch) · [AliExpress](https://www.aliexpress.com/wholesale?SearchText=2.8+inch+ILI9341+SPI+320x240+non+touch) |
| 2 — Compact | **2.4-inch ILI9341 TFT** | 320×240 SPI | Smallest/lightest top lid while keeping the same resolution | Very low; same ILI9341 driver, but verify offsets/rotation | Easier hinge and lower power; smaller text | [Amazon HiLetgo](https://www.amazon.com/HiLetgo-Display-ILI9341-Touch-240x320/dp/B07WNLNRDN) · [AliExpress](https://www.aliexpress.com/wholesale?SearchText=2.4+inch+ILI9341+SPI+240x320+non+touch) |
| 3 — Larger, same driver | **3.2-inch ILI9341 TFT** | 320×240 SPI | Bigger readable radar without changing controller family | Low; same driver, but case and rotation must be retested | Wider/heavier lid; same pixel count as 2.8-inch, so not sharper | [Amazon DIANN](https://www.amazon.com/DIANN-ILI9341-Display-320x240-Screen/dp/B0BNQD38T2) · [AliExpress](https://www.aliexpress.com/wholesale?SearchText=3.2+inch+ILI9341+SPI+240x320) |
| 4 — IPS upgrade | **2.0-inch ST7789V IPS** | 320×240 SPI | Best viewing angles, crisp compact status screen | Moderate; change driver/setup from ILI9341 to ST7789 and verify color order/offset | Smaller than selected screen; visually better from angled lid positions | [Amazon ST7789V](https://www.amazon.com/WatangTech-LCD-Display-Resolution-Controller/dp/B0FBKYFQ4Q) · [AliExpress](https://www.aliexpress.com/wholesale?SearchText=2.0+inch+ST7789+IPS+320x240+SPI) |
| 5 — Maximum size | **3.5-inch ILI9488 IPS/TFT** | 480×320 SPI | Largest radar/map view | High; different driver, larger framebuffer and slower full-screen SPI updates | Heaviest lid and highest power; needs stronger hinge and careful backlight power | [Amazon Hosyond](https://www.amazon.com/Hosyond-Display-Compatible-Mega2560-Development/dp/B0BWJHK4M6) · [Amazon IPS alternative](https://www.amazon.com/480x320-Display-Module-ILI9488-Driver/dp/B09318RGTV) · [AliExpress](https://www.aliexpress.com/wholesale?SearchText=3.5+inch+ILI9488+SPI+480x320+IPS) |

### Recommendation order

1. **Buy the 2.8-inch ILI9341 non-touch SPI module** for the planned build.
2. Choose the 2.4-inch ILI9341 only if pocket size and hinge weight matter more than visibility.
3. Choose the 3.2-inch ILI9341 if you want a visibly larger screen with the least software disruption.
4. Choose the 2.0-inch ST7789 IPS if viewing angle and image quality matter more than size.
5. Choose the 3.5-inch ILI9488 only for a desk-oriented cyberdeck; it is the least pocketable and requires the most firmware and enclosure work.

### Complete top-screen purchase list

- [ ] One selected SPI display module; **2.8-inch ILI9341 non-touch recommended**.
- [ ] One 8-position 2.54 mm female connector housing/header or equivalent detachable locking connector.
- [ ] Crimp contacts and a matching crimping tool, or pre-crimped female leads.
- [ ] Seven colors of flexible 28 AWG stranded wire.
- [ ] Heat-shrink tubing in small sizes.
- [ ] Fine-tip soldering iron, electronics solder, and flux.
- [ ] Digital multimeter for continuity and 3.3 V checks.
- [ ] M2 or M2.5 screws and heat-set brass inserts sized for the final enclosure.
- [ ] Two small hinges or one printed living/pin hinge rated for the screen weight.
- [ ] Lid stop, friction hinge, or kickstand so the display cable is not the angle stop.
- [ ] Thin foam tape or printed bezel supports; do not press directly on the LCD glass.
- [ ] Small strain-relief clamp where the seven-wire harness crosses the hinge.
- [ ] Optional right-angle USB-C data cable/adapter for flashing after assembly.
- [ ] Optional external 5 V backlight supply or switched backlight circuit for the 3.5-inch screen; do not assume the Cardputer 3.3 V rail can safely supply every large module.

### Listing checks before ordering

The listing must explicitly say:

- SPI serial interface.
- Correct controller: ILI9341, ST7789V, or ILI9488 as listed above.
- Correct resolution.
- Logic compatible with 3.3 V.
- A breakout PCB with accessible VCC, GND, SCK/CLK, MOSI/SDI, CS, DC/RS, and RST pins.
- Module dimensions that fit the enclosure.

Avoid listings that say only **Arduino UNO shield**, **8-bit parallel**, **16-bit parallel**, or **MCU interface** without confirming SPI. Product titles are sometimes inconsistent, so verify the pin labels in the product photos before purchasing.


## Gaming build from Short HPsiM36x3RA

Source: [Why You SHOULDN'T Use a Cardputer for Gaming](https://www.youtube.com/shorts/HPsiM36x3RA)

This Short tests the same Cardputer ADV dual-screen cyberdeck as a portable gaming device. It does **not** introduce a second top-screen design or require a new processor. The practical project addition is a removable gaming firmware setup for Doom and retro-console emulation.

### Honest capability summary

- Doom, NES, Game Boy/Color, Master System, Game Gear, and several other older systems can run on the Cardputer ADV.
- The Cardputer keyboard is usable, but it is not as comfortable as a real directional pad for long sessions.
- The selected 2.8-inch ILI9341 top screen will not automatically work with existing gaming firmware. Most released emulators draw to the built-in ST7789 display and require source changes to target the external TFT.
- SNES and Mega Drive support varies by title and may have slowdown, sound issues, tearing, or control limitations.
- Do not buy a different Cardputer or screen solely for this Short; use the ADV and 2.8-inch ILI9341 already selected.

### Gaming parts to buy

| Qty | Item | Requirement | Expected price |
|---:|---|---|---:|
| 1 | microSD card | 16–32 GB, name-brand, FAT32; 32 GB preferred | $7–$12 |
| 1 | USB-C data cable | Data-capable, not charge-only | $6–$12 |
| 1 | Optional M5Stack Joystick | Unit Joystick v1.1 (U024-C) or Joystick2 (U024-V2); verify connector/firmware support | $10–$18 |
| 1 | Optional printed D-pad cap | Cardputer key overlay; use a community model and verify ADV fit | $2–$8 printed |
| — | Legally obtained ROMs/WAD | Dumped from games you own or freely licensed homebrew | $0 project allowance |

The microSD card is the only new part normally required if the Cardputer ADV and cable are already owned.

### Correct gaming downloads

1. **M5Launcher** — install from the official project/release channel and use it to switch compatible firmware without repeatedly using a computer.
2. **Cardputer Game Station** — https://github.com/geo-tp/Cardputer-Game-Station-Emulators
3. **Cardputer Game Station releases** — https://github.com/geo-tp/Cardputer-Game-Station-Emulators/releases
4. **Cardputer ADV Doom keyboard-fixed port** — https://github.com/MAXXTANG/CardPuterAdvancedDoom
5. **Game Boy Enhanced firmware with Cardputer ADV support** — https://github.com/Mr-PauI/Gameboy-Enhanced-Firmware-m5stack-cardputer-
6. **M5Burner** — https://docs.m5stack.com/en/download

Do not use an older original-Cardputer Doom binary on the ADV just because it displays correctly. The original Cardputer keyboard uses different hardware; incompatible builds can show the demo while ignoring every key. Use the ADV-specific keyboard-fixed port.

### Gaming setup sequence

1. Back up anything important from the Cardputer and SD card.
2. Format a 16–32 GB microSD card as FAT32.
3. Install M5Launcher or use the exact flashing procedure documented by the selected firmware release.
4. First test Cardputer Game Station on the built-in screen.
5. Put only legally obtained, uncompressed ROM files on the SD card. Keep fewer than 512 files in one folder.
6. Test keyboard controls, sound, save/load, exit, and battery behavior before installing the clamshell.
7. For Doom, use the Cardputer ADV-specific build and its documented partition/flashing procedure. Do not assume every Doom image is M5Launcher-compatible.
8. Add the optional D-pad cap or joystick only after confirming the chosen firmware supports it.
9. Treat external-screen gaming as a later source-code port. Do not permanently wire or cut the display PCB expecting released emulators to use it automatically.

### Fabrication and electronics tools

#### Required or strongly recommended

| Tool | Why it is needed | Typical budget |
|---|---|---:|
| Fine-tip temperature-controlled soldering iron | Harness and connector work | $25–$45 |
| Electronics solder and flux | Reliable low-temperature joints | $8–$15 |
| Digital multimeter | Continuity, ground, and 3.3 V verification | $15–$25 |
| Dupont/JST-style crimp tool | Detachable seven-wire display harness | $18–$30 |
| Wire stripper/cutter for 28 AWG | Clean small-wire preparation | $8–$15 |
| Precision screwdriver set | Cardputer and enclosure disassembly | $10–$18 |
| Rotary tool with cutoff/sanding bits | Trim only excess display PCB fiberglass if the exact case requires it | $30–$60 |
| Eye protection and dust mask/respirator | Required when cutting fiberglass PCB | $10–$20 |
| Small files and 400–800 grit sandpaper | Final enclosure/PCB edge fitting | $6–$12 |
| Cyanoacrylate glue plus activator | Tidy harness and non-structural fixture work | $10–$16 |
| Heat source for heat-shrink | Small heat gun preferred | $15–$25 |
| Digital caliper | Verify display board, bezel, screw, and hinge dimensions | $12–$25 |

#### Optional if ordering a finished print

- FDM 3D printer, PLA+/PETG filament, nozzle tools, and heat-set-insert tip.
- Bench power supply for current-limit testing.
- Helping hands/PCB holder and solder fume extractor.
- Flush cutters, tweezers, spudger, and kapton tape.

Do not cut the display PCB unless the product photos and continuity inspection show that the area contains no traces, components, antenna, or ground plane needed by the module. Fiberglass dust is hazardous; work outdoors or with proper extraction and PPE.

### Complete physical-build consumables

| Item | Typical budget |
|---|---:|
| 2.8-inch ILI9341 SPI display | $10–$18 |
| 2.54 mm connector/crimp assortment | $8–$15 |
| 28 AWG multicolor stranded wire | $8–$14 |
| Heat-shrink assortment | $6–$10 |
| M2/M2.5 screws and heat-set inserts | $7–$14 |
| Small hinges/hinge hardware and lid stop | $6–$15 |
| Foam mounting tape and strain relief | $5–$10 |
| Printed enclosure if outsourced | $15–$40 |
| 32 GB microSD card | $7–$12 |

Prices are planning ranges, not checkout quotes. Shipping and sales tax are not included.

## Price budget

| Scenario | What it assumes | Estimated subtotal |
|---|---|---:|
| Gaming only | Cardputer ADV already owned; add microSD and reuse cable | **$7–$24** |
| Dual-screen parts only | Cardputer and tools already owned; screen, harness, hardware, print, microSD | **$72–$148** |
| Full build with new Cardputer | Add official Cardputer ADV at $29.90 before shipping/tax | **$102–$178** |
| Starter tool kit | Buy the listed essential fabrication/electronics tools | **$152–$286** |
| Complete first-time build | New Cardputer, dual-screen parts, gaming storage, and starter tools | **$254–$464** |

### Recommended spending target

Budget **about $150 before tax/shipping** if you already own the Cardputer ADV but need the dual-screen parts plus a few basic tools. Budget **about $325 before tax/shipping** for a comfortable first-time build starting with no tools. Buy the screen, microSD, connector parts, and enclosure hardware first; do not buy the optional joystick, 3.5-inch display, or external backlight supply until the basic ADV gaming firmware and 2.8-inch display tests pass.
