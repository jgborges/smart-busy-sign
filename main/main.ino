#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "defines.h"

const short FW_MAJOR = 1;
const short FW_MINOR = 0;

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
  setupTime();
}

void loop() {
  MDNS.update();
  handleSleep();
  handleClient();
  handleBlinking();
  yield();
}

void reboot() {
  Serial.println("Reboot...");
  yield();
  ESP.restart();
}
