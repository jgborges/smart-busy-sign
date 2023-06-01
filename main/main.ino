/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

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

// Create an instance of the server
// specify the port to listen on as an argument
//WiFiServer server(80);

// Allocate the JSON document
//
// Inside the brackets, 200 is the capacity of the memory pool in bytes.
// Don't forget to change this value to match your JSON document.
// Use https://arduinojson.org/v6/assistant to compute the capacity.
StaticJsonDocument<300> doc;

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

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.hostname(hostname);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin("esp8266")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) { delay(1000); }
  }
  Serial.println("mDNS responder started");

  if (!NBNS.begin(hostname)) {
    Serial.println("Error setting up NetBIOS!");
    while (1) { delay(1000); }
  }
  Serial.println("NetBIOS started");
 
  // Print the IP address
  Serial.println(WiFi.localIP());

  // Start the server
  //server.begin();
  //Serial.println(F("Server started"));

  startWebServer();
}

void startWebServer() {
  webServer.on(F("/status"), handleStatus);
  webServer.on(F("/ping"), handlePing);
  webServer.onNotFound(handleNotFound);
  webServer.begin();
  Serial.println("HTTP web server started");
}

void handleStatus() {
  if (webServer.method() == HTTP_GET) {
    Serial.println(F("GET Status"));

    DynamicJsonDocument doc(1024);
    doc["device"] = ARDUINO_BOARD;
    doc["device_version"]   = ARDUINO_ESP8266_RELEASE;
    doc["device_id"] = ARDUINO_BOARD_ID;

    String message = "";
    serializeJsonPretty(doc, message);
    Serial.println(message);

    webServer.send(200, "application/json", message);
  } else {
    Serial.println(F("POST Status"));
    webServer.send(500, "application/json", "not implemented!");
  }
}

void handlePing() {
  Serial.println(F("Handle ping"));
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

  /*
  // Check if a client has connected
  WiFiClient client = server.accept();
  if (!client) { return; }
  Serial.println(F("new client"));

  client.setTimeout(5000);  // default is 1000

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(F("request: "));
  Serial.println(req);

  // Match the request
  int val;
  if (req.indexOf(F("/gpio/0")) != -1) {
    val = HIGH;
  } else if (req.indexOf(F("/gpio/1")) != -1) {
    val = LOW;
  } else {
    Serial.println(F("invalid request"));
    val = digitalRead(D1);
  }

  // Set LED according to the request
  digitalWrite(LED_BUILTIN, val);
  digitalWrite(D1, val);

  // read/ignore the rest of the request
  // do not client.flush(): it is for output only, see below
  while (client.available()) {
    // byte by byte is not very efficient
    client.read();
  }

  // Send the response to the client
  // it is OK for multiple small client.print/write,
  // because nagle algorithm will group them into one single packet
  client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now "));
  client.print((val) ? F("low") : F("high"));
  client.print(F("<br><br>Click <a href='http://"));
  client.print(WiFi.localIP());
  client.print(F("/gpio/1'>here</a> to switch LED GPIO on, or <a href='http://"));
  client.print(WiFi.localIP());
  client.print(F("/gpio/0'>here</a> to switch LED GPIO off.</html>"));

  // The client will actually be *flushed* then disconnected
  // when the function returns and 'client' object is destroyed (out-of-scope)
  // flush = ensure written data are received by the other side
  Serial.println(F("Disconnecting from client"));
  */

  webServer.handleClient();
  yield();
}
