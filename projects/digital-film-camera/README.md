# Digital Film Camera — Raspberry Pi + E‑Paper

Nothing-inspired build guide for recreating the removable digital-film camera concept shown in:

- YouTube: https://youtu.be/9giyTIqTx1w
- Original project reference: https://strangeinventionslab.com/product/digital-film-camera/

## What it does

1. Press shutter.
2. Raspberry Pi captures a photo.
3. Crop and resize to 200×200.
4. Convert to black / white / red / yellow.
5. Send image over SPI.
6. Wait for e-paper refresh to complete.
7. Remove cartridge; image remains without power.

## Core parts

- Raspberry Pi 3 Model A+ — about $25 reference price
- 1.54-inch 4-color e-paper — about $10.99 each
- OV5647 5 MP camera — about $10–$20
- Waveshare Li-polymer Battery HAT — about $18.99
- microSD — about $6–$10
- 3.7 V LiPo — about $10–$18
- headers, switch, wire, hardware, filament — about $20–$40

Estimated one-cartridge prototype: **$95–$125**.
Estimated six-cartridge build: **$156–$183**.

## Wiring

| E-paper | Pi BCM | Physical pin |
|---|---:|---:|
| VCC | 3.3 V | 1 or 17 |
| GND | GND | 6 |
| DIN / MOSI | GPIO10 | 19 |
| CLK / SCLK | GPIO11 | 23 |
| CS | GPIO8 | 24 |
| DC | GPIO25 | 22 |
| BUSY | GPIO24 | 18 |
| RST | GPIO17 | 11 |
| Shutter | GPIO21 + GND | 40 + GND |

> Verify the exact display revision and connector orientation before applying power. Keep GPIO logic at 3.3 V.

## Raspberry Pi setup

```bash
sudo apt update
sudo apt install -y python3-pil python3-numpy python3-gpiozero
sudo raspi-config
```

Enable SPI, reboot, test the camera, then install the e-paper driver matching the exact display module.

## Recommended build order

1. Bench-test Pi OS and camera.
2. Test one e-paper module directly.
3. Add shutter button.
4. Build one removable cartridge.
5. Verify connector orientation with a multimeter.
6. Refresh a real photo.
7. Add Battery HAT + LiPo.
8. Install everything into the enclosure.
9. Build the remaining cartridges only after cartridge #1 is reliable.

## Reference links

- Raspberry Pi 3 Model A+: https://www.raspberrypi.com/products/raspberry-pi-3-model-a-plus/
- Waveshare 1.54-inch four-color e-paper: https://www.waveshare.com/1.54inch-e-paper-g.htm?sku=30475
- Waveshare Battery HAT: https://www.waveshare.com/catalog/product/view/id/3844/s/li-polymer-battery-hat/category/37/
- Raspberry Pi docs: https://www.raspberrypi.com/documentation/
- Waveshare e-paper wiki: https://www.waveshare.com/wiki/1.54inch_e-Paper_Module_(G)

## PDF

A Nothing-inspired PDF version of this guide was generated separately in ChatGPT as `Digital_Film_Camera_NOTHING_Style_Guide.pdf`.

This repository entry contains original notes and diagrams derived for recreation and does not reproduce the creator's paid CAD/source package.
