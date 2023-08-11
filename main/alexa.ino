#define ESPALEXA_ASYNC
#include <Espalexa.h>
#include <EspalexaDevice.h>
#include <ESPAsyncWebSrv.h>
#include "defines.h"

Espalexa espalexa;
bool alexaInitialized = false; // prevents sign updates to try to call Alexa before WiFi is initialized

std::map<String, EspalexaDevice*> alexaDeviceMap = { };

void setupAlexaWebServer(AsyncWebServer* server);
void addAlexaDevice(String deviceName, EspalexaDeviceType devType);
bool handleAlexaRequest(AsyncWebServerRequest *request);
void alexaStateChanged(EspalexaDevice* dev);

void setupAlexaWebServer(AsyncWebServer* server) {
  espalexa.begin(server);
  alexaInitialized = true;
  Serial.println("Alexa server initialized");
}

void setupAlexaDevices() {
  Serial.println("Creating Alexa devices");
  std::map<String, bool> deviceMap;
  getDeviceMap(deviceMap);

  alexaDeviceMap.clear();
  for (auto& item: deviceMap) {
    String name = item.first;
    bool isRGB = item.second;
    EspalexaDeviceType devType = isRGB ? EspalexaDeviceType::color : EspalexaDeviceType::dimmable;
    addAlexaDevice(name, devType);
  }
}

void addAlexaDevice(String deviceName, EspalexaDeviceType devType) {
  EspalexaDevice* dev = new EspalexaDevice(deviceName, alexaStateChanged, devType);
  if (espalexa.addDevice(dev)) {
    alexaDeviceMap.insert({ deviceName, dev });
    Serial.println(" device created: name=" + dev->getName() + ", id=" + String(dev->getId()));
  }
}

bool handleAlexaRequest(AsyncWebServerRequest *request) {
  return espalexa.handleAlexaApiCall(request);
}

void handleAlexa() {
  espalexa.loop();
}

/**
 * Notifiy Alexa the state has changed
*/
void updateAlexaDevice(String deviceName, bool state, byte brightness) {
  if (!alexaInitialized) {
    return;
  }
  Serial.printf(" set alexa device '%s' to state: %s brightness: %d\n", deviceName.c_str(), state ? "ON" : "OFF", brightness);
  if (auto it = alexaDeviceMap.find(deviceName); it != alexaDeviceMap.end()) {
    EspalexaDevice* dev = it->second;
    dev->setState(state);
    if (state) dev->setValue(brightness);
  }
}

/**
 * Alexa wants to change the state
*/
void updateAlexaDevice(String deviceName, bool state, byte brightness, byte r, byte g, byte b) {
  if (!alexaInitialized) {
    return;
  }
  Serial.printf(" set alexa device '%s' to state: %s brightness: %d, color: (%d,%d,%d)\n", deviceName.c_str(), state ? "ON" : "OFF", brightness, r, g, b);
  // notifiy alexa the state has changed
  if (auto it = alexaDeviceMap.find(deviceName); it != alexaDeviceMap.end()) {
    EspalexaDevice* dev = it->second;
    dev->setState(state);
    if (state) {
      dev->setValue(brightness);
      dev->setColor(r, g, b);
    }
  }
}

void alexaStateChanged(EspalexaDevice* dev) {
  if (dev == nullptr || alexaDeviceMap.count(dev->getName()) == 0) {
    return;
  }
  Serial.printf("[From Alexa] Device #%d state: %s brightness: %d\n", dev->getId(), dev->getState() ? "ON" : "OFF", dev->getValue());
  String state = dev->getState() ? "on" : "off";
  String color = dev->getType() == EspalexaDeviceType::color ? rgbToColor(dev->getR(), dev->getG(), dev->getB()) : "";
  String intensity = String(dev->getValue());
  setPanelStatus(dev->getName(), state, color, intensity, "", 0);
}

String rgbToColor(byte r, byte g, byte b) {
  char macStr[8] = { 0 };
  sprintf(macStr, "#%02X%02X%02X", r, g, b);
  return String(macStr);
}