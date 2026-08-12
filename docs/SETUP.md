# EP2-chip setup

These instructions target an ESP32-WROOM-32 style board with an integrated 1.14-inch ST7789 135x240 SPI LCD.

## 1. Hardware pin map

Use this mapping for the integrated screen:

| Signal | GPIO |
|---|---:|
| MOSI | 23 |
| SCLK | 18 |
| CS | 15 |
| DC | 2 |
| RST | 4 |
| BL / backlight | 32 |

The BOOT button is GPIO 0 and can be used as a user button after the board has booted.

Do not assign GPIO 2 to another feature. It is the LCD data/command line.

## 2. Install Arduino IDE support

1. Install Arduino IDE 2.x.
2. Open **File > Preferences**.
3. Add the Espressif ESP32 Boards Manager URL if it is not already present.
4. Open **Tools > Board > Boards Manager**.
5. Install **esp32 by Espressif Systems**.
6. Select **ESP32 Dev Module** as the initial board target.
7. Select the serial port for the board.

If the board is not detected, make sure the USB cable supports data. Some variants use a CH340 USB-to-serial interface, so a CH340 driver may be required on systems that do not already include one.

## 3. Install TFT_eSPI

1. Open **Library Manager** in Arduino IDE.
2. Search for **TFT_eSPI** by Bodmer.
3. Install it.

TFT_eSPI keeps its display controller and pin configuration in a setup header rather than in each sketch.

## 4. Add the EP2 display configuration

This repository includes:

`config/EP2_ST7789_Setup.h`

Recommended method:

1. Locate your Arduino libraries folder.
2. Create a folder beside `TFT_eSPI` named `TFT_eSPI_Setups`.
3. Copy `EP2_ST7789_Setup.h` into that folder.
4. Open `TFT_eSPI/User_Setup_Select.h`.
5. Disable the currently active default `User_Setup.h` include.
6. Add this include:

```cpp
#include <../TFT_eSPI_Setups/EP2_ST7789_Setup.h>
```

7. Save the file and restart Arduino IDE if necessary.

Keeping the custom setup outside the TFT_eSPI library folder helps prevent a library update from erasing the EP2 configuration.

## 5. First upload

Open:

`examples/display_test/display_test.ino`

Compile and upload it.

Expected result:

- Backlight turns on.
- Screen initializes in landscape mode.
- EP2-CHIP text appears.
- The display cycles through several solid colors.

If the screen lights but stays blank, verify the TFT_eSPI setup selection and the pin map above.

If colors appear reversed, keep the current setup and try toggling the RGB/BGR option in `EP2_ST7789_Setup.h`.

If the image is inverted, change the `tft.invertDisplay(true)` call in the sketch to `false`.

## 6. Try the examples

### EP2 menu

`examples/ep2_menu/ep2_menu.ino`

Controls:

- Short press BOOT: move to the next menu item.
- Hold BOOT for about 0.7 seconds: open the highlighted item.
- Hold BOOT for about 2 seconds: return to the menu.

The menu demonstrates graphics, a simple clock page, Wi-Fi information and a matrix-style screen effect without reusing the LCD control pins.

### Wi-Fi spectrum

`examples/wifi_spectrum/wifi_spectrum.ino`

The sketch scans nearby 2.4 GHz Wi-Fi networks and displays the strongest observed signal level on each Wi-Fi channel. It does not join those networks and does not collect credentials.

## 7. Wi-Fi credentials

For examples that connect to your Wi-Fi, change only these two values in the sketch:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

Do not commit real Wi-Fi passwords to a public GitHub repository.

## 8. Upload troubleshooting

If upload fails:

1. Confirm **ESP32 Dev Module** and the correct serial port are selected.
2. Try a USB-A to USB-C data cable or another known-good data cable if the board is not enumerating correctly.
3. Press and hold **BOOT**, start the upload, then release BOOT when the upload begins if automatic bootloader entry fails.
4. Press **EN/RESET** after a successful upload if the sketch does not start automatically.

## 9. Source reference

The initial EP2 hardware investigation used the public repository:

`macobt-a11y/ESP32-1.14-LCD-135x240-st7789-ideaspark-clone`

That project demonstrated the display, graphics tests, a menu, a small game and Wi-Fi visualization on this style of board. EP2-chip keeps the useful hardware findings while using independently written example code.
