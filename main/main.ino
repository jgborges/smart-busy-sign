#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#ifndef STASSID
#define STASSID "Apto54"
#define STAPSK "MFG2iIWq4Korj"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
const char* hostname = "smart-busy-sign";
const unsigned int port = 88;
const short FW_MAJOR = 1;
const short FW_MINOR = 0;

// Create an instance of the web server
// specify the port to listen on as an argument
ESP8266WebServer webServer(port);

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
  pinMode(D1, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LED_BUILTIN, LOW);

  setupWifi();

  setupSign();

  setupWebServer();

}

void setupWebServer() {
  webServer.on(F("/status"), handleStatus);
  webServer.on(F("/ping"), handlePing);
  webServer.onNotFound(handleNotFound);
  webServer.begin();
  Serial.println("HTTP web server started");
}

void handleStatus() {
  if (webServer.method() == HTTP_GET) {
    Serial.println("GET Status");

    String message = getSignStatus();
    Serial.println(message);

    webServer.send(200, "application/json", message);
  } else {
    Serial.println("POST Status");
    webServer.send(500, "application/json", "not implemented!");
  }
}

void handlePing() {
  Serial.println("Handle ping");
  webServer.send(404, "text/plain", "ping!!\n\n");
}

void handleNotFound() {
  Serial.println(F("Handle not found"));
  String message = "Resource Not Found\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMethod: ";
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";
  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }
  webServer.send(404, "text/plain", message);
}

void loop() {
  MDNS.update();

  webServer.handleClient();
  yield();
}
