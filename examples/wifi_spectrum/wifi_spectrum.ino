#include <TFT_eSPI.h>
#include <WiFi.h>

TFT_eSPI tft = TFT_eSPI();

static const int BACKLIGHT_PIN = 32;
static const int CHANNELS = 14;
int strongestRssi[CHANNELS];

void drawFrame() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("EP2 WIFI SPECTRUM", 120, 4, 2);

  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("-30", 1, 30, 1);
  tft.drawString("-100", 1, 105, 1);
  tft.drawFastHLine(22, 118, 214, TFT_DARKGREY);
}

void scanChannels() {
  for (int i = 0; i < CHANNELS; ++i) strongestRssi[i] = -100;

  int count = WiFi.scanNetworks(false, true);
  for (int i = 0; i < count; ++i) {
    int channel = WiFi.channel(i);
    if (channel >= 1 && channel <= CHANNELS) {
      int rssi = WiFi.RSSI(i);
      if (rssi > strongestRssi[channel - 1]) {
        strongestRssi[channel - 1] = rssi;
      }
    }
  }
  WiFi.scanDelete();
}

void drawBars() {
  const int bottom = 117;
  const int maxHeight = 78;
  const int startX = 25;
  const int step = 15;
  const int barWidth = 11;

  tft.fillRect(22, 27, 218, 108, TFT_BLACK);
  tft.drawFastHLine(22, 118, 214, TFT_DARKGREY);

  for (int i = 0; i < CHANNELS; ++i) {
    int height = map(strongestRssi[i], -100, -30, 1, maxHeight);
    height = constrain(height, 1, maxHeight);
    int x = startX + i * step;

    uint16_t color = TFT_RED;
    if (strongestRssi[i] > -55) color = TFT_GREEN;
    else if (strongestRssi[i] > -75) color = TFT_YELLOW;

    tft.fillRect(x, bottom - height, barWidth, height, color);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawNumber(i + 1, x + barWidth / 2, 121, 1);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.invertDisplay(true);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(100);

  drawFrame();
}

void loop() {
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.drawString("SCANNING", 238, 17, 1);

  scanChannels();
  drawFrame();
  drawBars();

  delay(1200);
}
