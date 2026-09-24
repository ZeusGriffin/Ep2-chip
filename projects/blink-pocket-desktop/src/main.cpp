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
#include <time.h>
#include "secrets.h" // WIFI_STA_SSID / WIFI_STA_PASS -- gitignored, see include/secrets.h

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
// Header clock/date widget -- small mono text, top-right corner of the fixed
// header strip. Fades to black, swaps content, fades back in: shows the time
// most of the time, briefly shows the date every ~8s. NEVER draws a fake or
// guessed time -- stays hidden entirely until a real NTP sync succeeds.
//
// Font: FreeMono9pt7b, same as the rest of the metadata text in this file.
// This is a stand-in for "NAPOSTMONO" -- that name doesn't match any font
// bundled with TFT_eSPI/GFXFF and no font file was provided. Swap FONT_META
// (top of file) for a converted NAPOSTMONO .h once you hand me the real font
// file (TFT_eSPI's fontconvert tool turns a TTF into a compatible header).
//
// Time source: one-shot background Wi-Fi+NTP sync (startClockSync() /
// pollClockSync()), using include/secrets.h for credentials. Non-blocking --
// the launcher is interactive immediately; the clock just appears later once
// synced. Timezone hardcoded to US Central (America/Chicago, matches San
// Antonio, TX) via POSIX TZ string, which auto-handles the DST switch twice
// a year -- flag if that's wrong or you're traveling.
// ---------------------------------------------------------------------------
static const char *CLOCK_TZ = "CST6CDT,M3.2.0,M11.1.0"; // US Central w/ auto DST

enum class ClockSyncState : uint8_t { Idle, Connecting, WaitingForTime, Synced, Failed };
static ClockSyncState clockSyncState = ClockSyncState::Idle;
static uint32_t clockSyncStartMs = 0;
static bool timeSynced = false;
static const uint32_t WIFI_CONNECT_TIMEOUT_MS = 8000;
static const uint32_t NTP_WAIT_TIMEOUT_MS = 5000;

void startClockSync() {
  if (strlen(WIFI_STA_SSID) == 0) {
    Serial.println("[clock] no Wi-Fi creds in include/secrets.h -- clock widget stays hidden");
    clockSyncState = ClockSyncState::Failed;
    return;
  }
  Serial.printf("[clock] connecting to '%s' for NTP (background, non-blocking)...\n", WIFI_STA_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASS);
  clockSyncStartMs = millis();
  clockSyncState = ClockSyncState::Connecting;
}

void pollClockSync(uint32_t now) {
  if (clockSyncState == ClockSyncState::Connecting) {
    if (WiFi.status() == WL_CONNECTED) {
      configTzTime(CLOCK_TZ, "pool.ntp.org", "time.nist.gov");
      clockSyncStartMs = now;
      clockSyncState = ClockSyncState::WaitingForTime;
      Serial.println("[clock] Wi-Fi connected, waiting for NTP...");
    } else if (now - clockSyncStartMs >= WIFI_CONNECT_TIMEOUT_MS) {
      Serial.println("[clock] Wi-Fi connect timed out -- clock widget stays hidden");
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      clockSyncState = ClockSyncState::Failed;
    }
  } else if (clockSyncState == ClockSyncState::WaitingForTime) {
    struct tm ti;
    if (getLocalTime(&ti, 0)) { // 0ms = single non-blocking poll, not a wait loop
      timeSynced = true;
      clockSyncState = ClockSyncState::Synced;
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF); // free the radio; startOtaMode() sets its own mode when needed
      Serial.println("[clock] NTP synced");
    } else if (now - clockSyncStartMs >= NTP_WAIT_TIMEOUT_MS) {
      Serial.println("[clock] NTP timed out -- clock widget stays hidden");
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      clockSyncState = ClockSyncState::Failed;
    }
  }
}

enum class ClockContent : uint8_t { Time, Date };
static ClockContent clockContent = ClockContent::Time;
static uint32_t clockPhaseStartMs = 0;
static bool clockFadingIn = true;
static bool clockFadingOut = false;
static uint8_t clockLastBrightness = 255;
static char clockLastText[16] = "";

static const uint32_t CLOCK_SHOW_TIME_MS = 8000;
static const uint32_t CLOCK_SHOW_DATE_MS = 3000;
static const uint32_t CLOCK_FADE_MS = 500;
static const int16_t CLOCK_ZONE_W = 76; // clears from (width-CLOCK_ZONE_W) to width, inside the 30px header

uint16_t grayColor565(uint8_t brightness) {
  return ((brightness >> 3) << 11) | ((brightness >> 2) << 5) | (brightness >> 3);
}

void updateClockWidget(uint32_t now) {
  if (!timeSynced) return; // never render a fake/guessed time

  uint32_t elapsed = now - clockPhaseStartMs;
  uint8_t brightness = 255;

  if (clockFadingIn) {
    if (elapsed >= CLOCK_FADE_MS) {
      clockFadingIn = false;
      clockPhaseStartMs = now;
      brightness = 255;
    } else {
      brightness = (uint8_t)((elapsed * 255) / CLOCK_FADE_MS);
    }
  } else if (clockFadingOut) {
    if (elapsed >= CLOCK_FADE_MS) {
      clockFadingOut = false;
      clockContent = (clockContent == ClockContent::Time) ? ClockContent::Date : ClockContent::Time;
      clockFadingIn = true;
      clockPhaseStartMs = now;
      brightness = 0;
    } else {
      brightness = 255 - (uint8_t)((elapsed * 255) / CLOCK_FADE_MS);
    }
  } else {
    uint32_t dwell = (clockContent == ClockContent::Time) ? CLOCK_SHOW_TIME_MS : CLOCK_SHOW_DATE_MS;
    if (elapsed >= dwell) {
      clockFadingOut = true;
      clockPhaseStartMs = now;
    }
    brightness = 255;
  }

  time_t nowT;
  time(&nowT);
  struct tm ti;
  localtime_r(&nowT, &ti);
  char buf[16];
  if (clockContent == ClockContent::Time) {
    strftime(buf, sizeof(buf), "%H:%M", &ti);
  } else {
    strftime(buf, sizeof(buf), "%m/%d", &ti);
  }

  if (brightness == clockLastBrightness && strcmp(buf, clockLastText) == 0) return; // nothing changed, skip the SPI write
  clockLastBrightness = brightness;
  strncpy(clockLastText, buf, sizeof(clockLastText));

  tft.fillRect(tft.width() - CLOCK_ZONE_W, 0, CLOCK_ZONE_W, 30, TFT_BLACK);
  tft.setTextDatum(TR_DATUM);
  tft.setFreeFont(FONT_META);
  tft.setTextColor(grayColor565(brightness), TFT_BLACK);
  tft.drawString(buf, tft.width() - 6, 9);
  tft.setTextFont(1);
  tft.setTextDatum(MC_DATUM); // restore the datum every other draw call in this file assumes
}

// ---------------------------------------------------------------------------
// Minimal page navigation -- proves FORWARD/BACK/RESET actually change what's
// on screen, not just print a label. Placeholder page names only; real app
// content (AI Face, Wi-Fi Radar, Miner, etc.) is a later milestone.
// ---------------------------------------------------------------------------
static const char *PAGE_NAMES[] = {"HOME", "WIFI", "MINER", "SETTINGS", "ABOUT", "FACE"};
static const uint8_t PAGE_COUNT = sizeof(PAGE_NAMES) / sizeof(PAGE_NAMES[0]);
#define FACE_PAGE_INDEX 5 // appended after the 5 existing approved pages -- their indices/order are untouched
static uint8_t currentPage = 0;
static bool pageOpened = false; // SELECT opens/closes the current page; FORWARD/BACK/RESET close it
static bool faceLive = false;   // true only when pageOpened && currentPage == FACE_PAGE_INDEX

// `face` must exist before drawPage() below (it calls face.reset() when the
// FACE page opens); drawFace() itself is defined later, only called from loop().
#include <FaceMotion.h>
static FaceMotion face;

void drawPage(uint8_t page) {
  const int16_t top = 31, bottom = tft.height() - 60;

  tft.fillRect(0, top, tft.width(), bottom - top, TFT_BLACK); // leave header rule + gesture strip alone
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  if (pageOpened && page == FACE_PAGE_INDEX) {
    // Live AI Face -- chrome drawn once here; drawFace() repaints the face
    // itself every loop() tick on top of this static frame. 1 tap cycles
    // through Idle/Listening/Thinking/Speaking for a hands-off demo since
    // there's no live AI backend driving these states yet.
    faceLive = true;
    face.reset(millis());
    tft.setFreeFont(FONT_META);
    tft.drawString("1 tap: next state", tft.width() / 2, top + 14);
    tft.drawString("3 taps: close", tft.width() / 2, bottom - 14);
    tft.setTextFont(1);
  } else if (pageOpened) {
    faceLive = false;
    // Framed "open" state -- a distinct boxed panel, not just the browsing view.
    tft.drawRect(12, top + 10, tft.width() - 24, bottom - top - 20, TFT_WHITE);
    tft.setFreeFont(FONT_DISPLAY);
    tft.drawString(PAGE_NAMES[page], tft.width() / 2, tft.height() / 2 - 20);
    tft.setFreeFont(FONT_META);
    tft.drawString("OPEN", tft.width() / 2, tft.height() / 2 + 12);
    tft.drawString("3 taps: close", tft.width() / 2, bottom - 14);
    tft.setTextFont(1);
  } else {
    faceLive = false; // covers SELECT closing FACE back to plain browsing
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
// AI Face (BUILD_PLAN Phase 1, item 5) -- wires the already-tested,
// hardware-neutral FaceMotion state machine (firmware/core/FaceMotion.*, see
// FaceMotion_TESTED.md) into an actual page. Minimal vector face only: two
// rounded-rect eyes (height scaled by eyeOpen, offset by gaze), a small
// rounded-rect mouth scaled by mouthOpen. Monochrome, no clutter -- matches
// BUILD_PLAN's "Nothing-style black/white/gray" visual language, same as
// every other page in this file.
// (FaceMotion.h and `face` are already declared above, before drawPage().)
// ---------------------------------------------------------------------------
void drawFace(const FaceFrame &f) {
  const int16_t zoneTop = 33, zoneBottom = 154; // clear of the header and the "3 taps: close" hint
  const int16_t cx = tft.width() / 2;
  const int16_t cy = (zoneTop + zoneBottom) / 2 + (int16_t)(f.bob * 150.0f);
  const int16_t eyeSpacing = 46, eyeW = 30, eyeHmax = 30;

  tft.fillRect(0, zoneTop, tft.width(), zoneBottom - zoneTop, TFT_BLACK);

  int16_t eyeH = (int16_t)(eyeHmax * f.eyeOpen);
  if (eyeH < 3) eyeH = 3; // never fully vanish -- reads as a blink, not a glitch
  int16_t gx = (int16_t)(f.gazeX * 10.0f);
  int16_t gy = (int16_t)(f.gazeY * 8.0f);

  tft.fillRoundRect(cx - eyeSpacing - eyeW / 2 + gx, cy - eyeH / 2 + gy, eyeW, eyeH, 6, TFT_WHITE);
  tft.fillRoundRect(cx + eyeSpacing - eyeW / 2 + gx, cy - eyeH / 2 + gy, eyeW, eyeH, 6, TFT_WHITE);

  int16_t mouthW = 26 + (int16_t)(f.mouthOpen * 20.0f);
  int16_t mouthH = 3 + (int16_t)(f.mouthOpen * 14.0f);
  tft.fillRoundRect(cx - mouthW / 2, cy + 34, mouthW, mouthH, mouthH / 2, TFT_WHITE);
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

  startClockSync(); // fires WiFi.begin() and returns immediately -- launcher above is already interactive
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
      } else if (faceLive) {
        // FORWARD is a no-op while any other page is open (unchanged) --
        // reused here, only for the live face, to cycle the demo expression.
        static const BlinkFaceState demoCycle[] = {
            BlinkFaceState::Idle, BlinkFaceState::Listening,
            BlinkFaceState::Thinking, BlinkFaceState::Speaking};
        static uint8_t demoIdx = 0;
        demoIdx = (demoIdx + 1) % 4;
        face.setState(demoCycle[demoIdx], now);
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
      faceLive = false;
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

  if (faceLive) {
    drawFace(face.update(now)); // non-blocking, self-timed inside FaceMotion
  }

  pollClockSync(now);   // no-op once Synced or Failed
  updateClockWidget(now); // no-op until timeSynced; redraws only on actual change

  // Heartbeat so we can confirm it's alive and not blocking, per BUILD_PLAN
  // non-blocking-UI rule -- nothing here yet, just a serial pulse.
  static uint32_t lastBeat = 0;
  if (now - lastBeat > 2000) {
    lastBeat = now;
    Serial.println("alive");
  }
}
