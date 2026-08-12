#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

static const int BACKLIGHT_PIN = 32;

void showMessage(const char* title, const char* detail, uint16_t background) {
  tft.fillScreen(background);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, background);
  tft.drawString(title, 120, 48, 4);
  tft.setTextColor(TFT_YELLOW, background);
  tft.drawString(detail, 120, 82, 2);
}

void setup() {
  Serial.begin(115200);

  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.invertDisplay(true);

  showMessage("EP2-CHIP", "ST7789 READY", TFT_BLACK);
  Serial.println("EP2-chip display initialized");
  delay(1800);
}

void loop() {
  showMessage("DISPLAY TEST", "RED", TFT_RED);
  delay(900);
  showMessage("DISPLAY TEST", "GREEN", TFT_GREEN);
  delay(900);
  showMessage("DISPLAY TEST", "BLUE", TFT_BLUE);
  delay(900);
  showMessage("EP2-CHIP", "DISPLAY OK", TFT_BLACK);
  delay(1500);
}
