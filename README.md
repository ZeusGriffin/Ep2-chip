# Ep2-chip

Starter firmware and hardware notes for the ESP32 1.14-inch 135x240 ST7789 LCD board used as the EP2-chip base.

## What is included

- `docs/SETUP.md` — step-by-step Arduino IDE and TFT_eSPI setup.
- `config/EP2_ST7789_Setup.h` — TFT_eSPI configuration for the integrated display.
- `examples/display_test/display_test.ino` — first-boot display test.
- `examples/wifi_spectrum/wifi_spectrum.ino` — Wi-Fi channel strength display.
- `examples/ep2_menu/ep2_menu.ino` — small one-button EP2 menu demo.

## Board / display mapping

| Function | ESP32 GPIO |
|---|---:|
| TFT MOSI | 23 |
| TFT SCLK | 18 |
| TFT CS | 15 |
| TFT DC | 2 |
| TFT RST | 4 |
| TFT Backlight | 32 |
| BOOT / user button | 0 |

Display: ST7789, 135x240 pixels, SPI.

> Important: GPIO 2 is reserved for the LCD DC line in this setup. Do not also use it as a general LED output while the display is active.

## Start here

1. Read `docs/SETUP.md`.
2. Install the ESP32 board package and `TFT_eSPI` in Arduino IDE.
3. Add `config/EP2_ST7789_Setup.h` to TFT_eSPI using the instructions in the setup guide.
4. Upload `examples/display_test/display_test.ino` first.
5. If the display test works, try the menu or spectrum example.

## Reference

Hardware behavior and feature ideas were cross-checked against:

`macobt-a11y/ESP32-1.14-LCD-135x240-st7789-ideaspark-clone`

That repository does not currently declare a license, so EP2-chip does not copy its source files verbatim. The examples here are independently written around the board pinout and observed functionality.
