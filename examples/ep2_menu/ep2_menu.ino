#include <TFT_eSPI.h>
#include <WiFi.h>
#include "time.h"

TFT_eSPI tft = TFT_eSPI();

static const int BACKLIGHT_PIN = 32;
static const int BUTTON_PIN = 0;

const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

enum ScreenState {
  MENU,
  MATRIX_SCREEN,
  CLOCK_SCREEN,
  WIFI_SCREEN
};

ScreenState state = MENU;
int selected = 0;
const char* items[] = {"MATRIX", "CLOCK", "WIFI INFO"};
const int itemCount = 3;

bool buttonDown = false;
unsigned long buttonStarted = 0;
unsigned long lastClockDraw = 0;

void drawMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_BLACK, TFT_CYAN);
  tft.fillRect(0, 0, 240, 30, TFT_CYAN);
  tft.drawString("EP2-CHIP", 120, 15, 4);

  for (int i = 0; i < itemCount; ++i) {
    int y = 50 + i * 25;
    if (i == selected) {
      tft.fillRoundRect(18, y - 9, 204, 19, 4, TFT_WHITE);
      tft.setTextColor(TFT_BLACK, TFT_WHITE);
    } else {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.drawString(items[i], 120, y, 2);
  }

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("tap: next   hold: open", 120, 126, 1);
}

void openSelected() {
  state = static_cast<ScreenState>(selected + 1);
  tft.fillScreen(TFT_BLACK);
  lastClockDraw = 0;
}

void returnToMenu() {
  state = MENU;
  drawMenu();
}

void handleButton() {
  bool pressed = digitalRead(BUTTON_PIN) == LOW;

  if (pressed && !buttonDown) {
    buttonDown = true;
    buttonStarted = millis();
  }

  if (!pressed && buttonDown) {
    unsigned long duration = millis() - buttonStarted;
    buttonDown = false;

    if (state == MENU) {
      if (duration < 500) {
        selected = (selected + 1) % itemCount;
        drawMenu();
      } else if (duration >= 650) {
        openSelected();
      }
    } else if (duration >= 1500) {
      returnToMenu();
    }
  }
}

void drawMatrix() {
  static unsigned long lastDraw = 0;
  if (millis() - lastDraw < 35) return;
  lastDraw = millis();

  int x = random(0, 24) * 10;
  int y = random(0, 13) * 10;
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(x, y);
  tft.print((char)random(33, 126));
}

void drawClock() {
  if (millis() - lastClockDraw < 500) return;
  lastClockDraw = millis();

  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("EP2 CLOCK", 120, 25, 4);

  struct tm timeInfo;
  if (getLocalTime(&timeInfo, 100)) {
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeInfo);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString(buffer, 120, 72, 7);
  } else {
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.drawString("NO TIME SYNC", 120, 72, 2);
  }

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("hold BOOT to return", 120, 122, 1);
}

void drawWifiInfo() {
  static unsigned long lastDraw = 0;
  if (millis() - lastDraw < 750) return;
  lastDraw = millis();

  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("WIFI INFO", 120, 24, 4);

  if (WiFi.status() == WL_CONNECTED) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(WiFi.localIP().toString(), 120, 62, 2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString(String(WiFi.RSSI()) + " dBm", 120, 88, 2);
  } else {
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.drawString("NOT CONNECTED", 120, 70, 2);
  }

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("hold BOOT to return", 120, 122, 1);
}

void setup() {
  Serial.begin(115200);

  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  tft.init();
  tft.setRotation(1);
  tft.invertDisplay(true);

  if (WIFI_SSID[0] != '\0') {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  }

  drawMenu();
}

void loop() {
  handleButton();

  switch (state) {
    case MENU:
      break;
    case MATRIX_SCREEN:
      drawMatrix();
      break;
    case CLOCK_SCREEN:
      drawClock();
      break;
    case WIFI_SCREEN:
      drawWifiInfo();
      break;
  }
}
