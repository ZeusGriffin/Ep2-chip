#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

WebServer server(8787);
String nodeState = "idle";

void sendStatus() {
  JsonDocument doc;
  doc["ok"] = true;
  doc["device"] = "BLINK NODE";
  doc["platform"] = "esp32";
  doc["state"] = nodeState;
  doc["battery"] = -1;
  doc["version"] = "0.1.0";
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void commandRoute() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"missing body\"}");
    return;
  }
  JsonDocument doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"bad json\"}");
    return;
  }
  const char* action = doc["action"] | "";
  if (!strcmp(action, "listen")) nodeState = "listening";
  else if (!strcmp(action, "stop") || !strcmp(action, "wake")) nodeState = "idle";
  else if (!strcmp(action, "sleep")) nodeState = "sleeping";
  else if (strcmp(action, "ping") && strcmp(action, "status") &&
           strcmp(action, "led") && strcmp(action, "launch")) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"unsupported action\"}");
    return;
  }
  sendStatus();
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(250);
  server.on("/api/v1/status", HTTP_GET, sendStatus);
  server.on("/api/v1/command", HTTP_POST, commandRoute);
  server.begin();
}

void loop() {
  server.handleClient();
}