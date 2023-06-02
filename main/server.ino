const unsigned int port = 80;

// Create an instance of the web server
// specify the port to listen on as an argument
ESP8266WebServer webServer(port);

void setupWebServer() {
  webServer.on(F("/"), handleHome);
  webServer.on(F("/status"), handleStatus);
  webServer.on(F("/about"), handleAbout);
  webServer.on(F("/admin/wifi"), handleAdminWifi);
  webServer.on(F("/admin/factory-reset"), handleAdminReset);
  webServer.on(F("/admin/sign-info"), handleAdminReset);
  webServer.onNotFound(handleNotFound);
  webServer.begin();
  Serial.println("HTTP web server started");
}

void handleClient() {
  webServer.handleClient();
}

void handleHome() {

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

void handleAbout() {
  Serial.println("Handle ping");
  webServer.send(404, "text/plain", "ping!!\n\n");
}

void handleAdminWifi() {
  Serial.println("Handle ping");
  webServer.send(404, "text/plain", "ping!!\n\n");
}

void handleAdminReset() {
  
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