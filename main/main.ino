#include <ArduinoJson.h>
#include <algorithm>
#include <map>
#include <array>
#include <vector>
#include "defines.h"
#include "sign.h"
#include "storage.h"

const short FW_MAJOR = 1;
const short FW_MINOR = 2;

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
  setupAlexaDevices();
  setupTime();
}

void loop() {
  handleMDNS();
  handleSleep();
  handleBlinking();
  handleTTL();
  handleAlexa();
  yield();
}

void reboot() {
  Serial.println("Reboot...");
  delay(100);
  yield();
  ESP.restart();
}
