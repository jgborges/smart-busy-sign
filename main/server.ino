#include "index.html.h"  // indexHtml

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
  webServer.on(F("/admin/sign-info"), handleAdminInfo);
  webServer.onNotFound(handleNotFound);
  webServer.begin();
  Serial.println("HTTP web server started");
}

void handleClient() {
  webServer.handleClient();
}

void handleHome() {
  if (webServer.method() == HTTP_GET) {
    Serial.println("Handle GET /");
    webServer.send(200, "text/html", indexHtml);
  }
}

void handleStatus() {
  if (webServer.method() == HTTP_GET) {
    Serial.println("Handle GET /status");

    String message = getSignStatus();
    Serial.println("GET Response");
    Serial.println(message);

    webServer.send(200, "application/json", message);
  } else {
    Serial.println("Handle POST /status");
    printRequestArgs();
    if (!webServer.hasArg("plain")) {
      webServer.send(400, "application/json", "no body");
      return;
    }
    
    String body = webServer.arg("plain");
    Serial.println("Request body: " + body);
    ParsingError err = setSignStatus(body);
    switch (err) {
      case SUCCESS: 
        webServer.send(200, "application/json", "successfully updated sign panels");
        break;
      case JSON_PARSING_ERROR:
        webServer.send(400, "application/json", "invalid payload");
        break;
      case JSON_MISSING_PANELS_FIELD:
      case JSON_MISSING_NAME_FIELD:
      case JSON_MISSING_STATE_FIELD:
        webServer.send(400, "application/json", "missing required field");
      default:
        webServer.send(500, "application/json", "unknown exception"); 
        break;
    }
  }
}

void handleAbout() {
  if (webServer.method() == HTTP_GET) {
    Serial.println("Handle GET /about");
    webServer.send(200, "text/plain", "under contruction!!\n\n");
  } else {
    Serial.println("Method " + HttpMethodToString(webServer.method()) + " /about");
    webServer.send(405, "text/plain", "Method not allowed");
  }
}

void handleAdminWifi() {
  if (webServer.method() == HTTP_GET) {
    Serial.println("Handle GET /admin/wifi");

    String message = getWifiStorage();
    Serial.println("GET Response");
    Serial.println(message);
    webServer.send(200, "application/json", message);
  } else {
    Serial.println("Handle POST /admin/wifi");
    String message = printRequestArgs();
    webServer.send(501, "text/plain", message);
    //webServer.send(501, "text/plain", "not supported\n\n");
  }
}

void handleAdminReset() {
  Serial.println("Handle /admin/factory-reset");
  webServer.send(501, "text/plain", "not supported\n\n");
}

void handleAdminInfo() {
  Serial.println("Handle /admin/sign-info");
  webServer.send(501, "text/plain", "not supported\n\n");
}

void handleNotFound() {
  Serial.println(F("Handle not found"));
  String message = printRequestArgs();
  webServer.send(404, "text/plain", message);
}

String printRequestArgs() {
  String message = HttpMethodToString(webServer.method()) + " args were:\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";
  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }
  Serial.print(message);
  return message;
}

String HttpMethodToString(const HTTPMethod method) {
  switch (method) {
    case HTTP_GET:
      return "GET";
    case HTTP_POST:
      return "POST";
    case HTTP_PATCH:
      return "PATCH";
    case HTTP_PUT:
      return "PUT";
    case HTTP_ANY:
      return "ANY";
    case HTTP_DELETE:
      return "DELETE";
    default:
      return String(method);
  }
}