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
