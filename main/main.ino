#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

const short FW_MAJOR = 1;
const short FW_MINOR = 0;

enum ParsingResult { 
  PARSE_SUCCESS, 
  NO_PANEL_FOUND,
  JSON_PARSING_ERROR, 
  JSON_MISSING_PANELS_FIELD, 
  JSON_MISSING_NAME_FIELD, 
  JSON_MISSING_STATE_FIELD,
};

enum PostResult { 
  POST_SUCCESS, 
  BAD_REQUEST,
  SERVER_ERROR
};

struct PanelSetup {
  String name;
  String color;
  uint8_t gpio;
};

struct PanelStatus {
  String name;
  String state;
  String color;
  byte intensity;
  ulong ttl;
};

bool staConnectionFailed = false;

void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.print(F("Firmware Version: "));
  Serial.print(FW_MAJOR);
  Serial.print(F("."));
  Serial.print(FW_MINOR);
  Serial.println();

  // print board info
  Serial.print(F("Board name: "));
  Serial.println(ARDUINO_BOARD);
  Serial.print(F("Board id: "));
  Serial.println(ARDUINO_BOARD_ID);
  Serial.print(F("Board version: "));
  Serial.println(ARDUINO_ESP8266_RELEASE);

  setupSign();
  setupStorage();
  setupWifi();
  setupWebServer();
}

void loop() {
  MDNS.update();
  handleClient();
  handleBlinking();
  yield();
}

void reboot() {
  Serial.println("Reboot...");
  yield();
  ESP.restart();
}
