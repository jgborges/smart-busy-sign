#include "index.html.h"
#include "resources/images.h"
#include <ESPAsyncWebSrv.h>

const unsigned int port = 80;

AsyncWebServer webServer(port);

class OneParamRewrite : public AsyncWebRewrite
{
  protected:
    String _urlPrefix;
    int _paramIndex;
    String _paramsBackup;

  public:
  OneParamRewrite(const char* from, const char* to)
    : AsyncWebRewrite(from, to) {

      _paramIndex = _from.indexOf('{');

      if (_paramIndex >=0 && _from.endsWith("}")) {
        _urlPrefix = _from.substring(0, _paramIndex);
        int index = _params.indexOf('{');
        if(index >= 0) {
          _params = _params.substring(0, index);
        }
      } else {
        _urlPrefix = _from;
      }
      _paramsBackup = _params;
  }

  bool match(AsyncWebServerRequest *request) override {
    if (request->url().startsWith(_urlPrefix)) {
      if (_paramIndex >= 0) {
        _params = _paramsBackup + request->url().substring(_paramIndex);
      } else {
        _params = _paramsBackup;
      }
      return true;
    } else {
      return false;
    }
  }
};

void handleRequestBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
bool getPostBody(AsyncWebServerRequest *request, String& body);
void handleHomeGet(AsyncWebServerRequest *request);
void handleResourcesGet(AsyncWebServerRequest *request);
void handleStatusGet(AsyncWebServerRequest *request);
void handleStatusPost(AsyncWebServerRequest *request);
void handleAdminDeviceGet(AsyncWebServerRequest *request);
void handleAdminDevicePost(AsyncWebServerRequest *request);
void handleAdminWifiGet(AsyncWebServerRequest *request);
void handleAdminWifiPost(AsyncWebServerRequest *request);
void handleAdminWifiScanGet(AsyncWebServerRequest *request);
void handleAdminSettingsGet(AsyncWebServerRequest *request);
void handleAdminSettingsPost(AsyncWebServerRequest *request);
void handleAdminRebootPost(AsyncWebServerRequest *request);
void handleAdminSleepPost(AsyncWebServerRequest *request);
void handleAdminResetPost(AsyncWebServerRequest *request);
void handleNotFound(AsyncWebServerRequest *request);
String printRequestArgs(AsyncWebServerRequest *request);
String httpMethodToString(const WebRequestMethodComposite method);

void setupWebServer() {
  webServer.on("/", HTTP_GET, handleHomeGet);
  webServer.on("/resources", HTTP_GET, handleResourcesGet);
  webServer.addRewrite(new OneParamRewrite("/resources/{id}", "/resources?id={id}"));
  webServer.on("/status", HTTP_GET, handleStatusGet);
  webServer.on("/status", HTTP_POST, handleStatusPost, NULL, handleRequestBody);
  webServer.on("/admin/device", HTTP_GET, handleAdminDeviceGet);
  webServer.on("/admin/device", HTTP_POST, handleAdminDevicePost, NULL, handleRequestBody);
  webServer.on("/admin/wifi", HTTP_GET, handleAdminWifiGet);
  webServer.on("/admin/wifi", HTTP_POST, handleAdminWifiPost, NULL, handleRequestBody);
  webServer.on("/admin/scan", HTTP_GET, handleAdminWifiScanGet);
  webServer.on("/admin/settings", HTTP_GET, handleAdminSettingsGet);
  webServer.on("/admin/settings", HTTP_POST, handleAdminSettingsPost, NULL, handleRequestBody);
  webServer.on("/admin/reboot", HTTP_POST, handleAdminRebootPost);
  webServer.on("/admin/sleep", HTTP_POST, handleAdminSleepPost, NULL, handleRequestBody);
  webServer.on("/admin/factory-reset", HTTP_POST, handleAdminResetPost);
  webServer.onNotFound(handleNotFound);
  webServer.onRequestBody(handleRequestBody);   

  webServer.begin();
  Serial.println("HTTP web server started");
}

void handleClient() {
  //webServer.handleClient();
}

void handleRequestBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  if (total > 0 && request->_tempObject == NULL && total < 4*1024) { // you may use your own size instead of 10240
    request->_tempObject = malloc(total+1);
    ((uint8_t*)request->_tempObject)[total] = 0;
  }
  if (request->_tempObject != NULL) {
    memcpy((uint8_t*)(request->_tempObject) + index, data, len);
  }
}

bool getPostBody(AsyncWebServerRequest *request, String& body) {
  if (request->hasParam("body", true)) {
    body = request->getParam("body", true)->value();
    Serial.println("Request body: " + body);
    return true;
  } else if (request->_tempObject != NULL) {
    body = String((char*)(request->_tempObject));
    Serial.println("Request body: " + body);
    return true;
  } else {
    return false;
  }
}

void handleHomeGet(AsyncWebServerRequest *request) {
  if (request->method() == HTTP_GET) {
    Serial.println("Handle GET /");
    request->send_P(200, "text/html", indexHtml, NULL);
  }
}

void handleResourcesGet(AsyncWebServerRequest *request) {
  if (request->method() == HTTP_GET) {
    Serial.println("Handle GET /resources");
    if (!request->hasParam("id")) {
      request->send(400, "text/plain", "resource not specified");
      return;
    }
    String resourceId = request->getParam("id")->value();
    Serial.println("Resouce id: " + resourceId);
    long resourceSize;
    const unsigned char* resource = getImageResourceFromId(resourceId, resourceSize);
    if (resource != NULL) {
      auto response = request->beginResponse_P(200, "image/png", resource, resourceSize, NULL);
      response->addHeader("Cache-Control", "max-age=84600"); // 24h suggested cache
      request->send(response);
    } else {
      request->send(404, "text/plain", "resource not found!");
    }
  }
}

void handleStatusGet(AsyncWebServerRequest *request) {
  Serial.println("Handle GET /status");

  String message = getSignStatus();
  Serial.println("GET Response");
  Serial.println(message);

  request->send(200, "application/json", message);
}

void handleStatusPost(AsyncWebServerRequest *request) {
  Serial.println("Handle POST /status");
  String body;
  if (!getPostBody(request, body)) {
    request->send(400, "text/plain", "no body");
    return;
  }
  ParsingResult err = setSignStatus(body);
  switch (err) {
    case PARSE_SUCCESS: 
      request->send(200, "text/plain", "successfully updated sign panels");
      break;
    case JSON_PARSING_ERROR:
      request->send(400, "text/plain", "invalid payload");
      break;
    case JSON_MISSING_PANELS_FIELD:
    case JSON_MISSING_NAME_FIELD:
    case JSON_MISSING_STATE_FIELD:
      request->send(400, "text/plain", "missing required field");
    default:
      request->send(500, "text/plain", "unknown exception"); 
      break;
  }
}

void handleAdminDeviceGet(AsyncWebServerRequest *request) {
  if (request->method() == HTTP_GET) {
    Serial.println("Handle GET /admin/device");
    String message = getDeviceInformation();
    Serial.println("GET Response");
    Serial.println(message);
    request->send(200, "application/json", message);
  }
}

void handleAdminDevicePost(AsyncWebServerRequest *request) {
  Serial.println("Handle POST /admin/device");
  String body;
  if (!getPostBody(request, body)) {
    request->send(400, "text/plain", "no body");
    return;
  }

  PostResult result = setDeviceInformation(body);

  switch (result) {
    case POST_SUCCESS:
      request->send(200, "text/plain", "device information updated successfully!");
      break;
    case BAD_REQUEST:
      request->send(400, "text/plain", "bad request");
      break;
    case SERVER_ERROR:
    default:
      request->send(500, "text/plain", "unknown server error");
      break;
  }
}

void handleAdminWifiGet(AsyncWebServerRequest *request) {
  Serial.println("Handle GET /admin/wifi");
  String message = getWifiConfig();
  Serial.println("GET Response");
  Serial.println(message);
  request->send(200, "application/json", message);
}

void handleAdminWifiPost(AsyncWebServerRequest *request) {
  Serial.println("Handle POST /admin/wifi");
  String body;
  if (!getPostBody(request, body)) {
    request->send(400, "text/plain", "no body");
    return;
  }

  PostResult result = setWifiConfig(body);

  switch (result) {
    case POST_SUCCESS:
      request->send(200, "text/plain", "Wifi config updated successfully! Device will reboot now...");
      delay(5000);
      reboot();
      break;
    case BAD_REQUEST:
      request->send(400, "text/plain", "bad request");
      break;
    case SERVER_ERROR:
    default:
      request->send(500, "text/plain", "unknown server error");
      break;
  }
}

void handleAdminWifiScanGet(AsyncWebServerRequest *request) {
  int n = WiFi.scanComplete();
  if (n == WIFI_SCAN_FAILED) {
    WiFi.scanNetworks(true);
    request->send(202, "text/plain", "wifi scan started");
  } else if (n == WIFI_SCAN_RUNNING) {
    request->send(202, "text/plain", "wifi scan is still running");
    return;
  } else {
    String json = "[";
    for (int i = 0; i < n; ++i){
      if(i) json += ",";
      json += "{";
      json += "\"rssi\":"+String(WiFi.RSSI(i));
      json += ",\"ssid\":\""+WiFi.SSID(i)+"\"";
      json += ",\"bssid\":\""+WiFi.BSSIDstr(i)+"\"";
      json += ",\"channel\":"+String(WiFi.channel(i));
      json += ",\"secure\":"+String(WiFi.encryptionType(i));
      json += "}";
    }
    json += "]";
    WiFi.scanDelete();
    request->send(200, "application/json", json);
  }
}

void handleAdminSettingsGet(AsyncWebServerRequest *request) {
  Serial.println("Handle GET /admin/settings");
  String message = getUserSettings();
  Serial.println("GET Response");
  Serial.println(message);
  request->send(200, "application/json", message);
}

void handleAdminSettingsPost(AsyncWebServerRequest *request) {
  Serial.println("Handle POST /admin/settings");
  String body;
  if (!getPostBody(request, body)) {
    request->send(400, "text/plain", "no body");
    return;
  }

  PostResult result = setUserSettings(body);

  switch (result) {
    case POST_SUCCESS:
      request->send(200, "text/plain", "user settings updated successfully!");
      break;
    case BAD_REQUEST:
      request->send(400, "text/plain", "bad request");
      break;
    case SERVER_ERROR:
    default:
      request->send(500, "text/plain", "unknown server error");
      break;
  }
}

void handleAdminRebootPost(AsyncWebServerRequest *request) {
  Serial.println("Handle POST /admin/reboot");
  request->send(200, "text/plain", "rebooting device...\n\n");
  delay(2000);
  reboot();
}

void handleAdminSleepPost(AsyncWebServerRequest *request) {
  Serial.println("Handle POST /admin/sleep");

  long sleepTimeInMinutes = 0;
  String body;
  if (getPostBody(request, body)) {
    bool valid = parseJsonIntField(body, "value", sleepTimeInMinutes);
    if (!valid) {
      request->send(400, "text/plain", "bad request");
      return;
    }
  }

  request->send(200, "text/plain", "sleeping device...\n\n");
  delay(2000);
  deepSleep(sleepTimeInMinutes);
}

void handleAdminResetPost(AsyncWebServerRequest *request) {
  Serial.println("Handle /admin/factory-reset");
  factoryReset(false);
  request->send(200, "text/plain", "Successfuly reset to factory default! Device will reboot now...\n\n");
  delay(2000);
  reboot();
}

void handleNotFound(AsyncWebServerRequest *request) {
  Serial.println("Handle not found: " + request->url());
  String body = "";
  getPostBody(request, body);
  if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), body)) {
    Serial.println("Successfully processed Fauxmo request");
    return;
  }
  String message = printRequestArgs(request);
  request->send(404, "text/plain", message);
}

String printRequestArgs(AsyncWebServerRequest *request) {
  String message = httpMethodToString(request->method()) + " args were:\n";
  message += "URI: ";
  message += request->url();
  message += "\nArguments: ";
  message += request->params();
  message += "\n";
  for (uint8_t i = 0; i < request->params(); i++) {
    message += " " + request->getParam(i)->name() + ": " + request->getParam(i)->value() + "\n";
  }
  Serial.print(message);
  return message;
}

String httpMethodToString(const WebRequestMethodComposite method) {
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

bool parseJsonIntField(String jsonBody, String field, long& value) {
  if (jsonBody.isEmpty()) {
    return false;
  }
  DynamicJsonDocument doc(50);
  DeserializationError err = deserializeJson(doc, jsonBody);
  if (err) {
    Serial.print("Deserialization error: ");
    Serial.println(err.f_str());
    return false;  
  }
  JsonObject root = doc.as<JsonObject>();
  if (!root.containsKey("value")) {
    Serial.println("missing 'value' field");
    return false;
  }

  value = root["value"].as<long>();
  return true;
}
