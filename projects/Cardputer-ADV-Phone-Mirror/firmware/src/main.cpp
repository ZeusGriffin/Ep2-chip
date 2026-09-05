#include <Arduino.h>
#include <WiFi.h>
#include <M5Cardputer.h>

namespace {
constexpr char kApSsid[] = "CardputerMirror";
constexpr char kApPassword[] = "cardputer";  // 8+ chars required by WPA2
constexpr uint16_t kPort = 9000;
constexpr size_t kMaxFrameBytes = 120 * 1024;
constexpr uint32_t kReadTimeoutMs = 2500;

WiFiServer frameServer(kPort);
uint8_t* frameBuffer = nullptr;
size_t frameCapacity = 0;
uint32_t frameCount = 0;
uint32_t lastFpsAt = 0;
uint32_t framesAtLastFps = 0;
float fps = 0.0f;

bool readExact(WiFiClient& client, uint8_t* dst, size_t bytes, uint32_t timeoutMs) {
  size_t got = 0;
  uint32_t lastProgress = millis();
  while (got < bytes && client.connected()) {
    int available = client.available();
    if (available > 0) {
      const size_t want = min(bytes - got, static_cast<size_t>(available));
      const int n = client.read(dst + got, want);
      if (n > 0) {
        got += static_cast<size_t>(n);
        lastProgress = millis();
      }
    } else {
      if (millis() - lastProgress > timeoutMs) return false;
      delay(1);
    }
  }
  return got == bytes;
}

uint32_t readU32BE(const uint8_t* p) {
  return (static_cast<uint32_t>(p[0]) << 24) |
         (static_cast<uint32_t>(p[1]) << 16) |
         (static_cast<uint32_t>(p[2]) << 8) |
         static_cast<uint32_t>(p[3]);
}

void showWaitingScreen() {
  auto& d = M5Cardputer.Display;
  d.fillScreen(TFT_BLACK);
  d.setTextColor(TFT_WHITE, TFT_BLACK);
  d.setTextSize(1);
  d.setCursor(8, 8);
  d.println("CARDPUTER MIRROR");
  d.setTextColor(TFT_CYAN, TFT_BLACK);
  d.println();
  d.println("Wi-Fi: CardputerMirror");
  d.println("Pass:   cardputer");
  d.println("Host:   192.168.4.1:9000");
  d.println();
  d.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  d.println("Start the iPhone broadcast");
  d.println("when connected to this Wi-Fi.");
}

void showError(const char* message) {
  auto& d = M5Cardputer.Display;
  d.fillRect(0, 112, 240, 23, TFT_BLACK);
  d.setTextColor(TFT_RED, TFT_BLACK);
  d.setCursor(4, 116);
  d.print(message);
}

void updateFps() {
  const uint32_t now = millis();
  if (lastFpsAt == 0) {
    lastFpsAt = now;
    framesAtLastFps = frameCount;
    return;
  }
  const uint32_t elapsed = now - lastFpsAt;
  if (elapsed >= 1000) {
    fps = (frameCount - framesAtLastFps) * 1000.0f / elapsed;
    framesAtLastFps = frameCount;
    lastFpsAt = now;
    Serial.printf("frames=%lu fps=%.1f free_heap=%u\n",
                  static_cast<unsigned long>(frameCount), fps, ESP.getFreeHeap());
  }
}

void handleClient(WiFiClient& client) {
  // Protocol: 4 bytes ASCII "CMIR" + 4-byte big-endian JPEG length + JPEG bytes.
  uint8_t header[8];

  while (client.connected()) {
    M5Cardputer.update();

    if (!client.available()) {
      delay(1);
      continue;
    }

    if (!readExact(client, header, sizeof(header), kReadTimeoutMs)) break;
    if (memcmp(header, "CMIR", 4) != 0) {
      showError("Bad stream header");
      break;
    }

    const uint32_t len = readU32BE(header + 4);
    if (len == 0 || len > kMaxFrameBytes) {
      showError("Frame too large");
      break;
    }

    if (len > frameCapacity) {
      uint8_t* next = static_cast<uint8_t*>(realloc(frameBuffer, len));
      if (!next) {
        showError("Out of memory");
        break;
      }
      frameBuffer = next;
      frameCapacity = len;
    }

    if (!readExact(client, frameBuffer, len, kReadTimeoutMs)) break;

    // Sender already crops/resizes to 240x135. maxWidth/maxHeight keeps us safe
    // if a different sender supplies a slightly larger JPEG.
    const bool ok = M5Cardputer.Display.drawJpg(
        frameBuffer, len, 0, 0,
        M5Cardputer.Display.width(), M5Cardputer.Display.height());
    if (!ok) {
      showError("JPEG decode failed");
    } else {
      ++frameCount;
      updateFps();
    }
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(100);

  auto cfg = M5.config();
  M5Cardputer.begin(cfg);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setBrightness(160);
  showWaitingScreen();

  WiFi.mode(WIFI_AP);
  const bool apOk = WiFi.softAP(kApSsid, kApPassword);
  if (!apOk) {
    showError("Wi-Fi AP failed");
    return;
  }

  frameServer.setNoDelay(true);
  frameServer.begin();
  Serial.printf("CardputerMirror ready at %s:%u\n",
                WiFi.softAPIP().toString().c_str(), kPort);
}

void loop() {
  M5Cardputer.update();
  WiFiClient client = frameServer.available();
  if (client) {
    client.setNoDelay(true);
    Serial.printf("sender connected: %s\n", client.remoteIP().toString().c_str());
    handleClient(client);
    client.stop();
    Serial.println("sender disconnected");
    showWaitingScreen();
  }
  delay(2);
}
