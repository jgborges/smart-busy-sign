#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266NetBIOS.h>
  #include <ESP8266mDNS.h>
  #include <ESP8266WebServer.h>

  #define BOARD_MODEL ARDUINO_BOARD
  #define BOARD_ID ARDUINO_BOARD_ID
  #define BOARD_VERSION ARDUINO_ESP8266_RELEASE
#elif defined(ESP32)
  #include <WiFi.h>

  #define BOARD_MODEL ARDUINO_BOARD
  #define BOARD_ID ARDUINO_BOARD_ID
  #define BOARD_VERSION ARDUINO_ESP32_RELEASE
#endif

#include <ArduinoJson.h>
#include <algorithm>
#include "defines.h"

const short FW_MAJOR = 1;
const short FW_MINOR = 0;

ADC_MODE(ADC_VCC);

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
  Serial.println(BOARD_MODEL);
  Serial.print(F("Board id: "));
  Serial.println(BOARD_ID);
  Serial.print(F("Board version: "));
  Serial.println(BOARD_VERSION);

  setupSign();
  setupStorage();
  setupWifi();
  setupWebServer();
  setupTime();
  setupAlexa();
}

void loop() {
  MDNS.update();
  handleSleep();
  handleClient();
  handleBlinking();
  handleTTL();
  handleAlexa();
  yield();
}

bool isWakingFromDeepSleep() {
  return ESP.getResetInfoPtr()->reason == REASON_DEEP_SLEEP_AWAKE;
}

void reboot() {
  Serial.println("Reboot...");
  delay(100);
  yield();
  ESP.restart();
}
