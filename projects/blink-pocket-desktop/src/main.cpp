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
static const char *PAGE_NAMES[] = {"HOME", "WIFI", "MINER", "SETTINGS", "ABOUT", "FACE", "ZEMO"};
static const uint8_t PAGE_COUNT = sizeof(PAGE_NAMES) / sizeof(PAGE_NAMES[0]);
#define WIFI_PAGE_INDEX 1  // real Wi-Fi Radar content below, same "reuse FORWARD-while-open" pattern as FACE
#define ABOUT_PAGE_INDEX 4 // real System Info content below
#define FACE_PAGE_INDEX 5 // appended after the 5 existing approved pages -- their indices/order are untouched
#define ZEMO_PAGE_INDEX 6 // appended after FACE -- talks to the real Zemo/AnythingLLM brain over Wi-Fi
static uint8_t currentPage = 0;
static bool pageOpened = false; // SELECT opens/closes the current page; FORWARD/BACK/RESET close it
static bool faceLive = false;   // true only when pageOpened && currentPage == FACE_PAGE_INDEX
static bool radarLive = false;  // true only when pageOpened && currentPage == WIFI_PAGE_INDEX
static bool zemoLive = false;   // true only when pageOpened && currentPage == ZEMO_PAGE_INDEX

// Radar state must exist before drawPage() below (it sets radarState when the
// WIFI page opens); the actual scan/draw logic (updateRadar()) is defined
// later, only called from loop() -- same forward-decl pattern as `face`.
enum class RadarState : uint8_t { Idle, WaitingForClock, Scanning, Shown };
static RadarState radarState = RadarState::Idle;
static uint32_t radarStateMs = 0;
static const uint32_t RADAR_AUTO_REFRESH_MS = 6000;

// Zemo connectivity state -- must exist before drawPage() below (it checks
// zemoState when the ZEMO page opens); the real client logic (zemoSendMessage(),
// the background task, drawing) is defined later, only reached from loop()/
// the FORWARD gesture handler -- same forward-decl pattern as `face`/radar.
enum class ZemoState : uint8_t { NotConfigured, Idle, Sending, ShowingReply, Error };
static ZemoState zemoState = ZemoState::NotConfigured;
static char zemoReplyBuf[220] = "";
static char zemoErrorBuf[64] = "";

// `face` must exist before drawPage() below (it calls face.reset() when the
// FACE page opens); drawFace() itself is defined later, only called from loop().
#include <FaceMotion.h>
static FaceMotion face;

// Forward decl -- real body (needs ESP.*/WiFi.* calls) defined after drawPage(),
// same pattern as drawFace(): drawPage() only needs to call it, not define it.
void drawAboutContent();
void drawZemoContent(); // draws the face + current status/reply for the ZEMO page

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
    radarLive = false;
    zemoLive = false;
    face.reset(millis());
    tft.setFreeFont(FONT_META);
    tft.drawString("1 tap: next state", tft.width() / 2, top + 14);
    tft.drawString("3 taps: close", tft.width() / 2, bottom - 14);
    tft.setTextFont(1);
  } else if (pageOpened && page == WIFI_PAGE_INDEX) {
    // Live Wi-Fi Radar -- chrome drawn once here; updateRadar() (loop()) does
    // the actual async scan + result rendering into the zone below this.
    // Passive scan only -- SSID/RSSI/open-or-locked, no deauth/cracking/etc.
    faceLive = false;
    radarLive = true;
    zemoLive = false;
    radarState = RadarState::Idle; // updateRadar() kicks off the first scan next tick
    radarStateMs = millis();
    tft.setFreeFont(FONT_META);
    tft.drawString("passive scan only", tft.width() / 2, top + 14);
    tft.drawString("3 taps: close", tft.width() / 2, bottom - 14);
    tft.setTextFont(1);
  } else if (pageOpened && page == ABOUT_PAGE_INDEX) {
    // Real device info, drawn once here + refreshed on FORWARD (uptime/heap
    // change; chip/flash/MAC don't). Not animated -- no loop() polling needed.
    faceLive = false;
    radarLive = false;
    zemoLive = false;
    drawAboutContent();
    tft.setFreeFont(FONT_META);
    tft.drawString("1 tap: refresh", tft.width() / 2, top + 14);
    tft.drawString("3 taps: close", tft.width() / 2, bottom - 14);
    tft.setTextFont(1);
  } else if (pageOpened && page == ZEMO_PAGE_INDEX) {
    // Live Zemo (AnythingLLM) chat -- reuses the same face as FACE_PAGE_INDEX,
    // in a shorter zone (room left below for status/reply text). Real Wi-Fi
    // client, verified against AnythingLLM's own source -- see zemoSendMessage()
    // below. 1 tap sends a fixed test ping; stays inert if unconfigured or
    // while Lacey/AnythingLLM aren't actually running.
    faceLive = false;
    radarLive = false;
    zemoLive = true;
    face.reset(millis());
    zemoState = (strlen(ZEMO_HOST) == 0 || strlen(ZEMO_WORKSPACE_SLUG) == 0 || strlen(ZEMO_API_KEY) == 0)
                    ? ZemoState::NotConfigured
                    : ZemoState::Idle;
    drawZemoContent();
    tft.setFreeFont(FONT_META);
    tft.drawString("1 tap: ping zemo", tft.width() / 2, top + 14);
    tft.drawString("3 taps: close", tft.width() / 2, bottom - 14);
    tft.setTextFont(1);
  } else if (pageOpened) {
    faceLive = false;
    radarLive = false;
    zemoLive = false;
    // Framed "open" state -- a distinct boxed panel, not just the browsing view.
    tft.drawRect(12, top + 10, tft.width() - 24, bottom - top - 20, TFT_WHITE);
    tft.setFreeFont(FONT_DISPLAY);
    tft.drawString(PAGE_NAMES[page], tft.width() / 2, tft.height() / 2 - 20);
    tft.setFreeFont(FONT_META);
    tft.drawString("OPEN", tft.width() / 2, tft.height() / 2 + 12);
    tft.drawString("3 taps: close", tft.width() / 2, bottom - 14);
    tft.setTextFont(1);
  } else {
    faceLive = false; // covers SELECT closing FACE/WIFI/ABOUT/ZEMO back to plain browsing
    radarLive = false;
    zemoLive = false;
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
// zoneTop/zoneBottom let callers give it a shorter zone when the rest of the
// page needs room too (e.g. ZEMO's status/reply text below it) -- FACE's own
// call below passes the same 33/154 it always has, so its look is unchanged.
// Eyebrows added (simple rects, tied to eyeOpen) to better match the
// reference "eyes, eyebrows, mouth" face concept from Zemo by ZUZ.
// ---------------------------------------------------------------------------
void drawFace(const FaceFrame &f, int16_t zoneTop, int16_t zoneBottom) {
  const int16_t cx = tft.width() / 2;
  const int16_t cy = (zoneTop + zoneBottom) / 2 + (int16_t)(f.bob * 150.0f);
  const int16_t eyeSpacing = 46, eyeW = 30, eyeHmax = 30;

  tft.fillRect(0, zoneTop, tft.width(), zoneBottom - zoneTop, TFT_BLACK);

  int16_t eyeH = (int16_t)(eyeHmax * f.eyeOpen);
  if (eyeH < 3) eyeH = 3; // never fully vanish -- reads as a blink, not a glitch
  int16_t gx = (int16_t)(f.gazeX * 10.0f);
  int16_t gy = (int16_t)(f.gazeY * 8.0f);

  int16_t browY = cy - eyeH / 2 + gy - 10 - (int16_t)((1.0f - f.eyeOpen) * 4.0f); // dips slightly as eyes close
  tft.fillRoundRect(cx - eyeSpacing - eyeW / 2 + gx - 2, browY, eyeW + 4, 4, 2, TFT_WHITE);
  tft.fillRoundRect(cx + eyeSpacing - eyeW / 2 + gx - 2, browY, eyeW + 4, 4, 2, TFT_WHITE);

  tft.fillRoundRect(cx - eyeSpacing - eyeW / 2 + gx, cy - eyeH / 2 + gy, eyeW, eyeH, 6, TFT_WHITE);
  tft.fillRoundRect(cx + eyeSpacing - eyeW / 2 + gx, cy - eyeH / 2 + gy, eyeW, eyeH, 6, TFT_WHITE);

  int16_t mouthW = 26 + (int16_t)(f.mouthOpen * 20.0f);
  int16_t mouthH = 3 + (int16_t)(f.mouthOpen * 14.0f);
  tft.fillRoundRect(cx - mouthW / 2, cy + 34, mouthW, mouthH, mouthH / 2, TFT_WHITE);
}

// ---------------------------------------------------------------------------
// Zemo by ZUZ -- BLINK-side client for Zee's local AnythingLLM/Ollama brain
// (github.com/techjarves/Portable-AI-USB, runs off the "Lacey" USB drive).
//
// Endpoint verified tonight against the real, current AnythingLLM server
// source (github.com/Mintplex-Labs/anything-llm, server/endpoints/api/
// workspace/index.js line ~603 + server/index.js's app.use("/api", ...) and
// default-port lines) -- not guessed:
//   POST http://<ZEMO_HOST>:<ZEMO_PORT>/api/v1/workspace/<ZEMO_WORKSPACE_SLUG>/chat
//   Authorization: Bearer <ZEMO_API_KEY>      Content-Type: application/json
//   Body:  {"message":"...","mode":"chat","sessionId":"blink","reset":false}
//   Reply: JSON, flat -- .textResponse (success) or .error (failure)
//
// NOT tested end-to-end -- Lacey isn't plugged in and the installer has
// never been run, so no server exists to hit yet. This is real, compiled,
// verified-against-source client code, waiting for that server.
//
// ESP32 Arduino's HTTPClient has no async mode, and an LLM reply can take
// many seconds -- so the actual request runs on a FreeRTOS task pinned to
// core 0, keeping gestures/animations on core 1 responsive the whole time
// (this file's non-blocking rule, same reason the clock/radar are async).
//
// Uses a raw WiFiClient + hand-built HTTP/1.1 request, not the HTTPClient
// class -- HTTPClient unconditionally pulls in WiFiClientSecure/mbedTLS
// (for https:// support this local, plain-http:// API doesn't need), which
// pushed the compiled binary 7.6KB past the 1.25MB partition limit on first
// try. WiFiClient alone adds zero new flash footprint (already linked in via
// WiFi.h for the clock/radar/OTA).
//
// Only sends a fixed test message tonight (1 tap while the page is open) --
// free-text chat via the BLE keyboard needs a real text-entry mode (keycode
// ->ASCII table, on-screen editing, line-wrapped rendering) that doesn't
// exist yet and isn't something to improvise blind. Flagged, not guessed.
// ---------------------------------------------------------------------------
#include <WiFiClient.h>

static const char *ZEMO_TEST_MESSAGE = "Hey Zemo, BLINK here -- can you hear me?";
static TaskHandle_t zemoTaskHandle = nullptr;
static volatile bool zemoTaskDone = false;

// Pulls one JSON string field's value out of AnythingLLM's flat, single-level
// {"textResponse": "...", "error": null} response shape. Not a general JSON
// parser (no nesting, no escaped-quote handling) -- sufficient for that
// confirmed shape; flag if a real reply ever breaks this assumption.
bool extractJsonStringField(const String &json, const char *key, char *out, size_t outLen) {
  String pat = String("\"") + key + "\":\"";
  int start = json.indexOf(pat);
  if (start < 0) return false;
  start += pat.length();
  int end = json.indexOf('"', start);
  if (end < 0) end = json.length();
  int n = end - start;
  if (n >= (int)outLen) n = outLen - 1;
  if (n < 0) n = 0;
  json.substring(start, start + n).toCharArray(out, n + 1);
  return true;
}

void zemoTask(void *param) {
  const char *message = (const char *)param;

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASS); // same home Wi-Fi as the clock/radar
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
      vTaskDelay(pdMS_TO_TICKS(100)); // blocks this task only -- core 1 (loop()) keeps running
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    strncpy(zemoErrorBuf, "wifi connect failed", sizeof(zemoErrorBuf));
    zemoTaskDone = true;
    vTaskDelete(nullptr);
    return;
  }

  WiFiClient client;
  client.setTimeout(30000); // LLM replies can be slow -- generous, this is off the UI core
  if (!client.connect(ZEMO_HOST, ZEMO_PORT)) {
    strncpy(zemoErrorBuf, "connect failed", sizeof(zemoErrorBuf));
    zemoTaskDone = true;
    vTaskDelete(nullptr);
    return;
  }

  char body[320];
  snprintf(body, sizeof(body),
           "{\"message\":\"%s\",\"mode\":\"chat\",\"sessionId\":\"blink\",\"reset\":false}",
           message); // fixed message only tonight, so no JSON-escaping needed yet
  size_t bodyLen = strlen(body);

  char reqHeader[256];
  snprintf(reqHeader, sizeof(reqHeader),
           "POST /api/v1/workspace/%s/chat HTTP/1.1\r\n"
           "Host: %s:%d\r\n"
           "Authorization: Bearer %s\r\n"
           "Content-Type: application/json\r\n"
           "Content-Length: %u\r\n"
           "Connection: close\r\n\r\n",
           ZEMO_WORKSPACE_SLUG, ZEMO_HOST, ZEMO_PORT, ZEMO_API_KEY, (unsigned)bodyLen);
  client.print(reqHeader);
  client.print(body);

  String statusLine = client.readStringUntil('\n');
  bool ok200 = statusLine.indexOf("200") > 0;

  while (client.connected() || client.available()) { // skip headers to the blank line
    String line = client.readStringUntil('\n');
    if (line.length() <= 1) break; // "\r" alone (or nothing left)
  }

  String respBody;
  uint32_t readStart = millis();
  while ((client.connected() || client.available()) && millis() - readStart < 30000 && respBody.length() < 1024) {
    while (client.available() && respBody.length() < 1024) respBody += (char)client.read();
  }
  client.stop();

  if (!ok200) {
    snprintf(zemoErrorBuf, sizeof(zemoErrorBuf), "http error: %s", statusLine.c_str());
  } else if (!extractJsonStringField(respBody, "textResponse", zemoReplyBuf, sizeof(zemoReplyBuf))) {
    if (!extractJsonStringField(respBody, "error", zemoErrorBuf, sizeof(zemoErrorBuf))) {
      strncpy(zemoErrorBuf, "empty reply", sizeof(zemoErrorBuf));
    }
  }
  zemoTaskDone = true;
  vTaskDelete(nullptr);
}

void zemoSendMessage(const char *message) {
  if (zemoState == ZemoState::NotConfigured || zemoState == ZemoState::Sending) return;
  zemoReplyBuf[0] = 0;
  zemoErrorBuf[0] = 0;
  zemoTaskDone = false;
  zemoState = ZemoState::Sending;
  face.setState(BlinkFaceState::Thinking, millis());
  xTaskCreatePinnedToCore(zemoTask, "zemoTask", 8192, (void *)message, 1, &zemoTaskHandle, 0);
}

// Simple, bounded word-wrap -- no library, just enough for a short LLM reply
// in a small status zone. Truncates with "..." past maxLines.
void drawWrappedText(const char *text, int16_t cx, int16_t yStart, int16_t lineH, uint8_t maxLines, uint8_t maxCharsPerLine) {
  tft.setTextDatum(MC_DATUM);
  tft.setFreeFont(FONT_META);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  char line[40];
  const char *p = text;
  uint8_t drawn = 0;
  while (*p && drawn < maxLines) {
    uint8_t n = 0;
    const char *lastSpace = nullptr;
    const char *scan = p;
    while (*scan && n < maxCharsPerLine) {
      if (*scan == ' ') lastSpace = scan;
      scan++;
      n++;
    }
    uint8_t take = n;
    if (*scan && lastSpace) take = lastSpace - p; // break on the last space, not mid-word
    bool lastLine = (drawn == maxLines - 1);
    strncpy(line, p, take);
    line[take] = 0;
    p += take;
    while (*p == ' ') p++;
    if (lastLine && *p) { // more text than fits -- mark truncation
      if (take > (uint8_t)(maxCharsPerLine - 3)) take = maxCharsPerLine - 3;
      line[take] = 0;
      strcat(line, "...");
    }
    tft.drawString(line, cx, yStart + drawn * lineH);
    drawn++;
  }
  tft.setTextFont(1);
}

void drawZemoContent() {
  const int16_t faceTop = 40, faceBottom = 104, statusTop = 112, statusBottom = 156;
  drawFace(face.update(millis()), faceTop, faceBottom);
  tft.fillRect(0, statusTop, tft.width(), statusBottom - statusTop, TFT_BLACK);

  switch (zemoState) {
    case ZemoState::NotConfigured:
      tft.setTextDatum(MC_DATUM);
      tft.setFreeFont(FONT_META);
      tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
      tft.drawString("not configured", tft.width() / 2, (statusTop + statusBottom) / 2 - 8);
      tft.drawString("see include/secrets.h", tft.width() / 2, (statusTop + statusBottom) / 2 + 12);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextFont(1);
      break;
    case ZemoState::Idle:
      tft.setTextDatum(MC_DATUM);
      tft.setFreeFont(FONT_META);
      tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
      tft.drawString("ready", tft.width() / 2, (statusTop + statusBottom) / 2);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextFont(1);
      break;
    case ZemoState::Sending:
      tft.setTextDatum(MC_DATUM);
      tft.setFreeFont(FONT_META);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.drawString("sending...", tft.width() / 2, (statusTop + statusBottom) / 2);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextFont(1);
      break;
    case ZemoState::ShowingReply:
      drawWrappedText(zemoReplyBuf, tft.width() / 2, statusTop + 2, 16, 3, 22);
      break;
    case ZemoState::Error:
      tft.setTextDatum(MC_DATUM);
      tft.setFreeFont(FONT_META);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString(zemoErrorBuf, tft.width() / 2, (statusTop + statusBottom) / 2);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextFont(1);
      break;
  }
}

// Called every loop() tick while zemoLive. Keeps the face animating; once the
// background task finishes, reads the result and moves the state machine on.
void updateZemoPage(uint32_t now) {
  drawFace(face.update(now), 40, 104);

  if (zemoState == ZemoState::Sending && zemoTaskDone) {
    if (zemoReplyBuf[0]) {
      zemoState = ZemoState::ShowingReply;
      face.setState(BlinkFaceState::Speaking, now);
    } else {
      zemoState = ZemoState::Error;
      face.setState(BlinkFaceState::Idle, now);
    }
    const int16_t statusTop = 112, statusBottom = 156;
    tft.fillRect(0, statusTop, tft.width(), statusBottom - statusTop, TFT_BLACK);
    if (zemoState == ZemoState::ShowingReply) {
      drawWrappedText(zemoReplyBuf, tft.width() / 2, statusTop + 2, 16, 3, 22);
    } else {
      tft.setTextDatum(MC_DATUM);
      tft.setFreeFont(FONT_META);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString(zemoErrorBuf, tft.width() / 2, (statusTop + statusBottom) / 2);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextFont(1);
    }
  }
}

// ---------------------------------------------------------------------------
// Wi-Fi Radar (BUILD_PLAN Phase 1, item 6 / README's explicit spec) --
// PASSIVE scan only: SSID, RSSI (dBm), open-vs-locked. No deauth, cracking,
// credential capture, packet injection, or impersonation -- not built, not
// planned. Async scan (WiFi.scanNetworks(true)) so it never blocks loop().
// Coordinates with the clock widget's Wi-Fi use: waits if a clock sync is
// still connecting, and never fights startOtaMode()'s AP mode.
// ---------------------------------------------------------------------------
const int16_t RADAR_ZONE_TOP = 59, RADAR_ZONE_BOTTOM = 156; // clear of the two static chrome lines drawPage() drew

void drawRadarStatus(const char *msg) {
  tft.fillRect(0, RADAR_ZONE_TOP, tft.width(), RADAR_ZONE_BOTTOM - RADAR_ZONE_TOP, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setFreeFont(FONT_META);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString(msg, tft.width() / 2, (RADAR_ZONE_TOP + RADAR_ZONE_BOTTOM) / 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(1);
}

void drawRadarResults(int16_t n) {
  tft.fillRect(0, RADAR_ZONE_TOP, tft.width(), RADAR_ZONE_BOTTOM - RADAR_ZONE_TOP, TFT_BLACK);
  if (n <= 0) {
    drawRadarStatus("no networks found");
    return;
  }
  uint8_t shown = (n > 5) ? 5 : (uint8_t)n;
  tft.setTextDatum(TL_DATUM);
  tft.setFreeFont(FONT_META);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  for (uint8_t i = 0; i < shown; i++) {
    char ssidBuf[15];
    strncpy(ssidBuf, WiFi.SSID(i).c_str(), 14);
    ssidBuf[14] = 0;
    char line[40];
    snprintf(line, sizeof(line), "%-14s %4ld %s", ssidBuf, (long)WiFi.RSSI(i),
             (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "open" : "lock");
    tft.drawString(line, 10, RADAR_ZONE_TOP + i * 18);
  }
  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(1);
}

void updateRadar(uint32_t now) {
  switch (radarState) {
    case RadarState::Idle:
      if (otaActive) return; // never fight OTA's AP mode
      if (clockSyncState == ClockSyncState::Connecting || clockSyncState == ClockSyncState::WaitingForTime) {
        radarState = RadarState::WaitingForClock;
        drawRadarStatus("waiting for clock sync");
        return;
      }
      WiFi.mode(WIFI_STA);
      WiFi.scanNetworks(true); // async -- non-blocking
      radarState = RadarState::Scanning;
      radarStateMs = now;
      drawRadarStatus("scanning...");
      Serial.println("[radar] scan started");
      break;
    case RadarState::WaitingForClock:
      if (!(clockSyncState == ClockSyncState::Connecting || clockSyncState == ClockSyncState::WaitingForTime)) {
        radarState = RadarState::Idle; // clock resolved -- retry next tick
      }
      break;
    case RadarState::Scanning: {
      int16_t n = WiFi.scanComplete();
      if (n == WIFI_SCAN_FAILED) {
        drawRadarStatus("scan failed");
        radarState = RadarState::Shown;
        radarStateMs = now;
      } else if (n >= 0) {
        drawRadarResults(n);
        WiFi.scanDelete();
        radarState = RadarState::Shown;
        radarStateMs = now;
        Serial.printf("[radar] found %d networks\n", n);
      } // else still WIFI_SCAN_RUNNING -- keep waiting, no draw
      break;
    }
    case RadarState::Shown:
      if (now - radarStateMs >= RADAR_AUTO_REFRESH_MS) {
        radarState = RadarState::Idle; // auto re-scan
      }
      break;
  }
}

// ---------------------------------------------------------------------------
// System Info (BUILD_PLAN Phase 1, item 7) -- all real, live-queried values.
// No guessed/fake data: chip model + flash size come from the running ESP32
// itself, MAC from the radio, build timestamp from the compiler, uptime/heap
// computed live. Static (not animated) -- redrawn once on open + on FORWARD.
// ---------------------------------------------------------------------------
void drawAboutContent() {
  const int16_t zoneTop = 59, zoneBottom = 156;
  tft.fillRect(0, zoneTop, tft.width(), zoneBottom - zoneTop, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.setFreeFont(FONT_META);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  char line[40];
  uint32_t upS = millis() / 1000;
  int16_t y = zoneTop;

  snprintf(line, sizeof(line), "%s rev%d", ESP.getChipModel(), ESP.getChipRevision());
  tft.drawString(line, 10, y); y += 18;
  snprintf(line, sizeof(line), "flash %luKB", (unsigned long)(ESP.getFlashChipSize() / 1024));
  tft.drawString(line, 10, y); y += 18;
  snprintf(line, sizeof(line), "heap %luKB free", (unsigned long)(ESP.getFreeHeap() / 1024));
  tft.drawString(line, 10, y); y += 18;
  snprintf(line, sizeof(line), "up %luh %02lum %02lus", (unsigned long)(upS / 3600),
           (unsigned long)((upS / 60) % 60), (unsigned long)(upS % 60));
  tft.drawString(line, 10, y); y += 18;
  snprintf(line, sizeof(line), "%s", WiFi.macAddress().c_str());
  tft.drawString(line, 10, y); y += 18;
  snprintf(line, sizeof(line), "build %s %s", __DATE__, __TIME__);
  tft.drawString(line, 10, y);

  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(1);
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
      } else if (radarLive) {
        radarState = RadarState::Idle; // FORWARD = force a manual re-scan
      } else if (pageOpened && currentPage == ABOUT_PAGE_INDEX) {
        drawAboutContent(); // FORWARD = refresh uptime/heap
      } else if (zemoLive) {
        zemoSendMessage(ZEMO_TEST_MESSAGE); // FORWARD = ping Zemo (no-op if not configured/already sending)
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
      radarLive = false;
      zemoLive = false;
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
    drawFace(face.update(now), 33, 154); // non-blocking, self-timed inside FaceMotion
  }

  if (radarLive) {
    updateRadar(now); // async scan poll + auto-refresh, never blocks
  }

  if (zemoLive) {
    updateZemoPage(now); // face keeps animating; picks up the background task's result once done
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
