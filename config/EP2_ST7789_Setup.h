// EP2-chip TFT_eSPI setup
// Target: ESP32 + integrated 1.14-inch ST7789 135x240 LCD

#define USER_SETUP_INFO "EP2-chip ST7789 135x240"

#define ST7789_DRIVER

#define TFT_WIDTH  135
#define TFT_HEIGHT 240

// Try TFT_RGB if red/blue are swapped on your panel.
#define TFT_RGB_ORDER TFT_BGR

#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4
#define TFT_BL   32
#define TFT_BACKLIGHT_ON HIGH

// No MISO line is required for the integrated display.

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

// Conservative starting speed for reliable bring-up.
#define SPI_FREQUENCY 27000000
