// BLINK Pocket Desktop — Phase 1 Step 3: minimal display test
// Board: NM-TV-154 (ESP32-D0WD-V3, ST7789 240x240)
// Goal: prove the panel initializes and renders. Nothing else.
// See: hardware/NM-TV-154/pinmap.md for confirmed pin source.

#include <Arduino.h>
#include <TFT_eSPI.h>

// Real vector fonts (TFT_eSPI GFXFF, requires LOAD_GFXFF -- already enabled
// in platformio.ini) replacing the scaled 8x8 GLCD bitmap font, which looks
// aliased/cheap when stretched. Bold sans for display text, mono for
// metadata/status -- mirrors the reference brochure's own pairing of a bold
// grotesk headline face with a monospaced metadata face.
// NOTE: TFT_eSPI's own Fonts/GFXFF/gfxfont.h (pulled in by TFT_eSPI.h when
// LOAD_GFXFF is set) already #includes every bundled Free Font -- doing it
// again here causes "redefinition" errors. The font symbols are already
// global once <TFT_eSPI.h> is included; no separate #include needed.
#define FONT_DISPLAY &FreeSansBold18pt7b     // big page-name content -- plenty of vertical room there
#define FONT_WORDMARK &FreeSansBold12pt7b    // header strip is only 30px tall; 18pt would clip
#define FONT_META &FreeMono9pt7b
#define FONT_META_BOLD &FreeMonoBold9pt7b

TFT_eSPI tft = TFT_eSPI();

// LCD power rail enable (per hardware/NM-TV-154/pinmap.md).
// Not a TFT_eSPI-managed pin -- must be driven manually before tft.init(),
// or the panel has no power regardless of SPI/backlight state.
#define LCD_POWER_PIN 21

static const uint16_t testColors[] = {
    TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_WHITE, TFT_BLACK
};

// ---------------------------------------------------------------------------
// Touch gesture input (GPIO32 / T9) — the ONLY input on this board (confirmed:
// no touchscreen, one capacitive pad). Threshold source: RockBase-iot/NM-TV-154
// HAL docs (raw<90 = pressed). Starting value; recalibrate live if it misfires.
//   1 tap        -> FORWARD
//   2 taps        -> BACK
//   3 taps        -> SELECT
//   hold (800ms)  -> RESET (event only -- not yet wired to an actual action)
// ---------------------------------------------------------------------------
#define TOUCH_PIN T9
static const uint16_t TOUCH_THRESHOLD = 90;
static const uint32_t HOLD_MS = 800;
static const uint32_t TAP_WINDOW_MS = 400;

static bool touchDown = false;
static uint32_t touchDownMs = 0;
static bool holdFired = false;
static uint8_t tapCount = 0;
static uint32_t lastTapMs = 0;

enum GestureAction { GESTURE_NONE, GESTURE_FORWARD, GESTURE_BACK, GESTURE_SELECT, GESTURE_RESET, GESTURE_OTA };

bool touchPressed() {
  return touchRead(TOUCH_PIN) < TOUCH_THRESHOLD;
}

void showGesture(const char *label, uint16_t color) {
  tft.fillRect(0, tft.height() - 60, tft.width(), 60, TFT_BLACK);
  tft.setTextColor(color, TFT_BLACK);
  tft.setFreeFont(FONT_META_BOLD); // bold mono -- status line, not content
  tft.drawString(label, tft.width() / 2, tft.height() - 30);
  tft.setTextFont(1); // reset to built-in font for whatever draws next
  Serial.printf("[gesture] %s\n", label);
}

GestureAction pollGesture(uint32_t now) {
  bool pressed = touchPressed();
  GestureAction fired = GESTURE_NONE;

  if (pressed && !touchDown) {
    touchDown = true;
    touchDownMs = now;
    holdFired = false;
  } else if (pressed && touchDown && !holdFired && (now - touchDownMs) >= HOLD_MS) {
    holdFired = true;
    tapCount = 0; // a hold cancels any pending tap count
    fired = GESTURE_RESET;
  } else if (!pressed && touchDown) {
    touchDown = false;
    if (!holdFired) {
      tapCount++;
      lastTapMs = now;
    }
  } else if (!pressed && !touchDown && tapCount > 0 && (now - lastTapMs) >= TAP_WINDOW_MS) {
    if (tapCount == 1) fired = GESTURE_FORWARD;
    else if (tapCount == 2) fired = GESTURE_BACK;
    else if (tapCount >= 10) fired = GESTURE_OTA;
    else fired = GESTURE_SELECT; // 3-9 taps all collapse to SELECT
    tapCount = 0;
  }

  return fired;
}

// ---------------------------------------------------------------------------
// Wired input over the EXISTING USB-C/CH340 serial connection (the same wire
// already used to flash/monitor this board). This is NOT generic USB-HID
// keyboard support -- a plain USB keyboard cannot be plugged into this port;
// the ESP32-D0WD-V3 has no native USB host, and the Type-C port is hard-wired
// only to the CH340 programming chip (confirmed via schematic). This reads
// raw bytes sent over that same serial line instead.
//   f/F -> FORWARD   b/B -> BACK   s/S -> SELECT   r/R -> RESET
// ---------------------------------------------------------------------------
void pollWiredSerialInput() {
  while (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 'f': case 'F': showGesture("FORWARD (wired)", TFT_GREEN); break;
      case 'b': case 'B': showGesture("BACK (wired)", TFT_YELLOW); break;
      case 's': case 'S': showGesture("SELECT (wired)", TFT_CYAN); break;
      case 'r': case 'R': showGesture("RESET (wired)", TFT_RED); break;
      default: break; // ignore newlines / unmapped bytes
    }
  }
}

// ---------------------------------------------------------------------------
// Bluetooth (BLE) keyboard input -- NimBLE central/client role, connects to
// an external BLE keyboard and reads its standard HID "Boot Keyboard Input
// Report" (fixed 8-byte format: modifier, reserved, key1..key6). This is a
// Bluetooth SIG-standard characteristic (UUID 0x2A22 under HID service
// 0x1812), not board-specific -- works with any BLE keyboard exposing Boot
// Protocol (most do; report-protocol-only keyboards may not connect).
//   Enter -> SELECT   RightArrow -> FORWARD   LeftArrow -> BACK   Esc -> RESET
// ---------------------------------------------------------------------------
#include <NimBLEDevice.h>

static const NimBLEUUID HID_SERVICE_UUID((uint16_t)0x1812);
static const NimBLEUUID BOOT_KEYBOARD_INPUT_UUID((uint16_t)0x2A22);

static NimBLEAdvertisedDevice *bleTargetDevice = nullptr;
static bool bleScanTriggered = false;
static bool bleConnected = false;

#define HID_KC_ENTER 0x28
#define HID_KC_ESC   0x29
#define HID_KC_RIGHT 0x4F
#define HID_KC_LEFT  0x50

void onKeyboardNotify(NimBLERemoteCharacteristic *chr, uint8_t *data, size_t len, bool isNotify) {
  if (len < 3) return; // boot report is 8 bytes; need modifier+reserved+key1
  switch (data[2]) {
    case HID_KC_ENTER: showGesture("SELECT (BLE kb)", TFT_CYAN); break;
    case HID_KC_RIGHT: showGesture("FORWARD (BLE kb)", TFT_GREEN); break;
    case HID_KC_LEFT:  showGesture("BACK (BLE kb)", TFT_YELLOW); break;
    case HID_KC_ESC:   showGesture("RESET (BLE kb)", TFT_RED); break;
    default: break;
  }
}

class BleScanCallbacks : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice *device) override {
    if (device->haveServiceUUID() && device->isAdvertisingService(HID_SERVICE_UUID)) {
      Serial.printf("[BLE] Found HID device: %s\n", device->toString().c_str());
      NimBLEDevice::getScan()->stop();
      bleTargetDevice = new NimBLEAdvertisedDevice(*device);
    }
  }
};

bool connectToKeyboard() {
  if (!bleTargetDevice) return false;
  NimBLEClient *client = NimBLEDevice::createClient();
  if (!client->connect(bleTargetDevice)) {
    Serial.println("[BLE] connect failed");
    return false;
  }
  NimBLERemoteService *svc = client->getService(HID_SERVICE_UUID);
  if (!svc) { Serial.println("[BLE] no HID service"); return false; }
  NimBLERemoteCharacteristic *chr = svc->getCharacteristic(BOOT_KEYBOARD_INPUT_UUID);
  if (!chr) { Serial.println("[BLE] no boot keyboard report char"); return false; }
  if (!chr->subscribe(true, onKeyboardNotify)) {
    Serial.println("[BLE] subscribe failed");
    return false;
  }
  Serial.println("[BLE] keyboard connected + subscribed");
  return true;
}

void startBleKeyboardScan() {
  if (bleScanTriggered) return;
  bleScanTriggered = true;
  NimBLEDevice::init("BLINK-NMTV154");
  NimBLEScan *scan = NimBLEDevice::getScan();
  scan->setScanCallbacks(new BleScanCallbacks());
  scan->setActiveScan(true);
  Serial.println("[BLE] scanning for a keyboard (10s)...");
  scan->start(10000, false); // blocking 10s window, runs once after boot
  if (bleTargetDevice) {
    bleConnected = connectToKeyboard();
  }
}

// ---------------------------------------------------------------------------
// Wireless (WiFi) OTA update -- triggered by 10 taps. This is WiFi-based OTA
// (ArduinoOTA), NOT Bluetooth -- there is no standard BLE "DFU" on the
// Arduino-ESP32 stack; WiFi is the real wireless firmware-update transport.
// Runs its own access point so no home WiFi credentials are needed.
// ---------------------------------------------------------------------------
#include <WiFi.h>
#include <ArduinoOTA.h>

static const char *OTA_AP_SSID = "BLINK-OTA";
static const char *OTA_AP_PASS = "blinkflash";
static bool otaActive = false;

void startOtaMode() {
  if (otaActive) return;
  otaActive = true;

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  tft.setFreeFont(FONT_DISPLAY);
  tft.drawString("OTA MODE", tft.width() / 2, 45);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(OTA_AP_SSID, OTA_AP_PASS);
  IPAddress ip = WiFi.softAPIP();

  ArduinoOTA.setHostname("blink-nmtv154");
  ArduinoOTA.begin();

  tft.setFreeFont(FONT_META);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  char line1[40], line2[40];
  snprintf(line1, sizeof(line1), "WiFi: %s", OTA_AP_SSID);
  snprintf(line2, sizeof(line2), "IP: %s", ip.toString().c_str());
  tft.drawString(line1, tft.width() / 2, 100);
  tft.drawString(line2, tft.width() / 2, 122);
  tft.drawString("pass: blinkflash", tft.width() / 2, 144);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("WiFi OTA, not Bluetooth", tft.width() / 2, 175);
  tft.setTextFont(1);

  Serial.println("[OTA] WiFi AP + ArduinoOTA started");
  Serial.printf("[OTA] SSID=%s IP=%s\n", OTA_AP_SSID, ip.toString().c_str());
}

// ---------------------------------------------------------------------------
// Minimal page navigation -- proves FORWARD/BACK/RESET actually change what's
// on screen, not just print a label. Placeholder page names only; real app
// content (AI Face, Wi-Fi Radar, Miner, etc.) is a later milestone.
// ---------------------------------------------------------------------------
static const char *PAGE_NAMES[] = {"HOME", "WIFI", "MINER", "SETTINGS", "ABOUT"};
static const uint8_t PAGE_COUNT = sizeof(PAGE_NAMES) / sizeof(PAGE_NAMES[0]);
static uint8_t currentPage = 0;
static bool pageOpened = false; // SELECT opens/closes the current page; FORWARD/BACK/RESET close it

void drawPage(uint8_t page) {
  const int16_t top = 31, bottom = tft.height() - 60;

  tft.fillRect(0, top, tft.width(), bottom - top, TFT_BLACK); // leave header rule + gesture strip alone
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  if (pageOpened) {
    // Framed "open" state -- a distinct boxed panel, not just the browsing view.
    tft.drawRect(12, top + 10, tft.width() - 24, bottom - top - 20, TFT_WHITE);
    tft.setFreeFont(FONT_DISPLAY);
    tft.drawString(PAGE_NAMES[page], tft.width() / 2, tft.height() / 2 - 20);
    tft.setFreeFont(FONT_META);
    tft.drawString("OPEN", tft.width() / 2, tft.height() / 2 + 12);
    tft.drawString("3 taps: close", tft.width() / 2, bottom - 14);
    tft.setTextFont(1);
  } else {
    tft.setFreeFont(FONT_DISPLAY);
    tft.drawString(PAGE_NAMES[page], tft.width() / 2, tft.height() / 2 - 10);

    char idx[16];
    snprintf(idx, sizeof(idx), "%02d / %02d", page + 1, PAGE_COUNT); // zero-padded, matches metadata style in reference
    tft.setFreeFont(FONT_META);
    tft.drawString(idx, tft.width() / 2, tft.height() / 2 + 32);
    tft.setTextFont(1);
  }

  tft.drawFastHLine(0, bottom, tft.width(), TFT_WHITE); // rule line above gesture strip

  Serial.printf("[page] %s (%d/%d) opened=%d\n", PAGE_NAMES[page], page + 1, PAGE_COUNT, pageOpened);
}

// ---------------------------------------------------------------------------
// UI style pass -- monochrome editorial/grotesk direction (ref: Aeonik Fono
// type-specimen brochure). Cannot embed that actual commercial typeface;
// approximating the direction with fonts already in the toolchain instead:
// bold blocky headers, thin rule lines, monospace zero-padded metadata,
// high-contrast black/white, grid layout.
// ---------------------------------------------------------------------------
void drawHeader() {
  tft.fillRect(0, 0, tft.width(), 30, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(FONT_WORDMARK); // bold sans wordmark, not the scaled bitmap font
  tft.drawString("BLINK", 8, 2); // TL_DATUM: y is the top of the glyph box, sized to fit inside the 30px bar
  tft.setTextFont(1); // back to default GLCD font for everything else
  tft.setTextDatum(MC_DATUM);
  tft.drawFastHLine(0, 30, tft.width(), TFT_WHITE); // thin rule line under header
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("BLINK Pocket Desktop -- minimal display test (NM-TV-154)");

  pinMode(LCD_POWER_PIN, OUTPUT);
  digitalWrite(LCD_POWER_PIN, LOW);   // LOW = power rail ENABLED (confirmed: RockBase-iot/NM-TV-154 HAL docs)
  delay(50); // let the rail settle before driving SPI/backlight

  tft.init();
  tft.setRotation(0); // adjust later once physical orientation is confirmed
  tft.invertDisplay(true); // this ST7789 panel requires inversion for correct colors (confirmed: RockBase-iot/NM-TV-154 HAL docs)

  // Color sweep: proves full panel drives correctly, no tearing/garbage
  for (uint8_t i = 0; i < sizeof(testColors) / sizeof(testColors[0]); i++) {
    tft.fillScreen(testColors[i]);
    delay(350);
  }

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  tft.setFreeFont(FONT_DISPLAY);
  tft.drawString("NODE", tft.width() / 2, tft.height() / 2 - 25);

  tft.setFreeFont(FONT_META);
  tft.drawString("Made by ZUZ", tft.width() / 2, tft.height() / 2 + 15);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("DISPLAY OK", tft.width() / 2, tft.height() / 2 + 40);

  char info[48];
  snprintf(info, sizeof(info), "%dx%d rot%d", tft.width(), tft.height(), 0);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString(info, tft.width() / 2, tft.height() - 18);
  tft.setTextFont(1);

  Serial.printf("Display initialized: %dx%d\n", tft.width(), tft.height());

  startBleKeyboardScan(); // one-time 10s scan after display is confirmed up

  drawHeader();
  drawPage(currentPage); // enter the placeholder menu at page 0
}

void loop() {
  uint32_t now = millis();

  pollWiredSerialInput();

  GestureAction g = pollGesture(now);
  switch (g) {
    case GESTURE_FORWARD:
      if (!pageOpened) {
        currentPage = (currentPage + 1) % PAGE_COUNT;
        drawPage(currentPage);
      }
      showGesture("FORWARD (1 tap)", TFT_GREEN);
      break;
    case GESTURE_BACK:
      if (!pageOpened) {
        currentPage = (currentPage + PAGE_COUNT - 1) % PAGE_COUNT;
        drawPage(currentPage);
      }
      showGesture("BACK (2 taps)", TFT_YELLOW);
      break;
    case GESTURE_SELECT:
      pageOpened = !pageOpened;
      drawPage(currentPage);
      showGesture(pageOpened ? "OPEN (3 taps)" : "CLOSE (3 taps)", TFT_CYAN);
      break;
    case GESTURE_RESET:
      currentPage = 0;
      pageOpened = false;
      drawPage(currentPage);
      showGesture("RESET (hold)", TFT_RED);
      break;
    case GESTURE_OTA:
      startOtaMode();
      break;
    default:
      break;
  }

  if (otaActive) {
    ArduinoOTA.handle();
  }

  // Heartbeat so we can confirm it's alive and not blocking, per BUILD_PLAN
  // non-blocking-UI rule -- nothing here yet, just a serial pulse.
  static uint32_t lastBeat = 0;
  if (now - lastBeat > 2000) {
    lastBeat = now;
    Serial.println("alive");
  }
}
