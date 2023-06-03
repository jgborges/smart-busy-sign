#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

const short FW_MAJOR = 1;
const short FW_MINOR = 0;

struct PanelSetup {
  String name;
  String color;
  ushort gpio_pin;
};

struct PanelStatus {
  String name;
  String state;
  String color;
  ushort intensity;
};

void setup() {
  Serial.begin(115200);

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

  // prepare LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  setupStorage();

  setupWifi();

  setupSign();

  setupWebServer();

}

void loop() {
  MDNS.update();
  handleClient();
  yield();
}
