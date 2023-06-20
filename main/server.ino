#include "index.html.h"  // indexHtml
#include "resources/images.h"
#include <uri/UriBraces.h>

const unsigned int port = 80;

// Create an instance of the web server
// specify the port to listen on as an argument
ESP8266WebServer webServer(port);

void setupWebServer() {
  webServer.on(F("/"), HTTP_GET, handleHome);
  webServer.on(UriBraces("/resources/{}"), HTTP_GET, handleResources);
  webServer.on(F("/status"), handleStatus);
  webServer.on(F("/admin/device"), handleAdminDevice);
  webServer.on(F("/admin/wifi"), handleAdminWifi);
  webServer.on(F("/admin/reboot"), HTTP_POST, handleAdminReboot);
  webServer.on(F("/admin/factory-reset"), HTTP_POST, handleAdminReset);
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

void handleResources() {
  if (webServer.method() == HTTP_GET) {
    Serial.println("Handle GET /resources");
    String resourceId = webServer.pathArg(0);
    Serial.println("Resouce id: " + resourceId);
    long resourceSize;
    const unsigned char* resource = getImageResourceFromId(resourceId, resourceSize);
    if (resource != NULL) {
      webServer.sendHeader("Cache-Control", "max-age=84600", false); // 24h suggested cache
      webServer.send(200, "image/png", resource, resourceSize);
    } else {
      webServer.send(404, "text/plain", "resource not found!");
    }
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
    if (!webServer.hasArg("plain")) {
      webServer.send(400, "application/json", "no body");
      return;
    }
    
    String body = webServer.arg("plain");
    Serial.println("Request body: " + body);
    ParsingResult err = setSignStatus(body);
    switch (err) {
      case PARSE_SUCCESS: 
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

void handleAdminDevice() {
  if (webServer.method() == HTTP_GET) {
    Serial.println("Handle GET /admin/device");
    String message = getDeviceInformation();
    Serial.println("GET Response");
    Serial.println(message);
    webServer.send(200, "application/json", message);
  } else {
    Serial.println("Handle POST /admin/device");
    if (!webServer.hasArg("plain")) {
      webServer.send(400, "application/json", "no body");
      return;
    }
    String body = webServer.arg("plain");
    Serial.println("Request body: " + body);

    PostResult result = setDeviceInformation(body);

    switch (result) {
      case POST_SUCCESS:
        webServer.send(200, "text/html", "device information updated successfully!");
        break;
      case BAD_REQUEST:
        webServer.send(400, "text/plain", "bad request");
        break;
      case SERVER_ERROR:
      default:
        webServer.send(500, "text/plain", "unknown server error");
        break;
    }
  }
}

void handleAdminWifi() {
  if (webServer.method() == HTTP_GET) {
    Serial.println("Handle GET /admin/wifi");
    String message = getWifiConfig();
    Serial.println("GET Response");
    Serial.println(message);
    webServer.send(200, "application/json", message);
  } else {
    Serial.println("Handle POST /admin/wifi");
    if (!webServer.hasArg("plain")) {
      webServer.send(400, "application/json", "no body");
      return;
    }
    String body = webServer.arg("plain");
    Serial.println("Request body: " + body);

    PostResult result = setWifiConfig(body);

    switch (result) {
      case POST_SUCCESS:
        webServer.send(200, "text/html", "Wifi config updated successfully! Device will reboot now...");
        delay(5000);
        reboot();
        break;
      case BAD_REQUEST:
        webServer.send(400, "text/plain", "bad request");
        break;
      case SERVER_ERROR:
      default:
        webServer.send(500, "text/plain", "unknown server error");
        break;
    }
  }
}

void handleAdminReboot() {
  Serial.println("Handle /admin/reboot");
  webServer.send(200, "text/plain", "rebooting device...\n\n");
  delay(2000);
  reboot();
}

void handleAdminReset() {
  Serial.println("Handle /admin/factory-reset");
  factoryReset(false);
  webServer.send(200, "text/plain", "Successfuly reset to factory default! Device will reboot now...\n\n");
  delay(2000);
  reboot();
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