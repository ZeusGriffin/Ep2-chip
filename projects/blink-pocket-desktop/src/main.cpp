// BLINK Pocket Desktop — Phase 1 Step 3: minimal display test
// Board: NM-TV-154 (ESP32-D0WD-V3, ST7789 240x240)
// Goal: prove the panel initializes and renders. Nothing else.
// See: hardware/NM-TV-154/pinmap.md for confirmed pin source.

#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

static const uint16_t testColors[] = {
    TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_WHITE, TFT_BLACK
};

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("BLINK Pocket Desktop -- minimal display test (NM-TV-154)");

  tft.init();
  tft.setRotation(0); // adjust later once physical orientation is confirmed

  // Color sweep: proves full panel drives correctly, no tearing/garbage
  for (uint8_t i = 0; i < sizeof(testColors) / sizeof(testColors[0]); i++) {
    tft.fillScreen(testColors[i]);
    delay(350);
  }

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  tft.setTextSize(2);
  tft.drawString("NODE", tft.width() / 2, tft.height() / 2 - 20);

  tft.setTextSize(1);
  tft.drawString("Made by ZUZ", tft.width() / 2, tft.height() / 2 + 10);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("DISPLAY OK", tft.width() / 2, tft.height() / 2 + 30);

  char info[48];
  snprintf(info, sizeof(info), "%dx%d rot%d", tft.width(), tft.height(), 0);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString(info, tft.width() / 2, tft.height() - 15);

  Serial.printf("Display initialized: %dx%d\n", tft.width(), tft.height());
}

void loop() {
  // Heartbeat so we can confirm it's alive and not blocking, per BUILD_PLAN
  // non-blocking-UI rule -- nothing here yet, just a serial pulse.
  static uint32_t lastBeat = 0;
  if (millis() - lastBeat > 2000) {
    lastBeat = millis();
    Serial.println("alive");
  }
}
