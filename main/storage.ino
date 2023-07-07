#include <EEPROM.h>

#ifndef AP_SSID
#define AP_SSID "Smart-Busy-Sign"
#define AP_PSK "smartsign"
#endif

#define BR_TIMEZONE -3*60
#define MAX_TZ_OFFSET 11*60
#define MIN_TZ_OFFSET -11*60
#define MAX_AUTO_TURNOFF_PERIOD (ushort)(24*60)

DeviceInfo devInfo = {};
WifiConfig wifiConfig = {};
Settings settings = {};

#define STORAGE_SIZE    4096.
#define STORAGE_BLOCK   512
#define DEVICE_INFO_POS 0*STORAGE_BLOCK
#define WIFI_CONFIG_POS 1*STORAGE_BLOCK
#define SETTINGS_POS    2*STORAGE_BLOCK

void setupStorage() {
  loadDeviceInformation();
  loadWifiConfig();
  loadUserSettings();
}

void loadDeviceInformation() {
  Serial.print("Reading device information from EEPROM...");
  EEPROM.begin(STORAGE_SIZE);
  EEPROM.get(DEVICE_INFO_POS, devInfo);
  EEPROM.end();

  if (!devInfo.isSet()) {
    Serial.println("Failed. Using default");
    devInfo.setValue = 0;
    strncpy(devInfo.signModel, "smart-busy-sign", sizeof(devInfo.signModel));
    strncpy(devInfo.serialNumber, "0000000001", sizeof(devInfo.serialNumber));
    strncpy(devInfo.manufacturingDate, "2023-06-01", sizeof(devInfo.manufacturingDate));
  } else {
    Serial.println("Done");
  }

  Serial.println(" sign-model: " + String(devInfo.signModel));
  Serial.println(" serial-number: " + String(devInfo.serialNumber));
  Serial.println(" manufacturing-date: " + String(devInfo.manufacturingDate));
}

String getDeviceInformation() {
  DynamicJsonDocument doc(500);
  doc["signModel"] = String(devInfo.signModel);
  doc["serialNumber"] = String(devInfo.serialNumber);
  doc["manufacturingDate"] = String(devInfo.manufacturingDate);
  doc["boardModel"] = String(BOARD_MODEL);
  doc["boardVersion"] = String(BOARD_VERSION);
  doc["firmwareVersion"] = String(FW_MAJOR) + "." + String(FW_MINOR);
  doc["timestamp"] = getEpochTime();
  doc["uptime"] = getUptime();
  doc["inactiveTime"] = getInactiveTime();
  doc["inputVoltage"] = ESP.getVcc()/1000.0;
  DynamicJsonDocument memory(300); 
  memory["freHeapSize"] = ESP.getMaxFreeBlockSize();
  memory["heapFragmentation"] = ESP.getHeapFragmentation();
  memory["sketchSize"] = ESP.getSketchSize();
  memory["freeSketchSize"] = ESP.getFreeSketchSpace();
  doc["memory"] = memory;

  String message = "";
  serializeJsonPretty(doc, message);
  return message;
}

PostResult setDeviceInformation(String jsonBody) {
  if (jsonBody.isEmpty()) {
    return BAD_REQUEST;
  }
  DynamicJsonDocument doc(250);
  DeserializationError err = deserializeJson(doc, jsonBody);
  if (err) {
    Serial.print("Deserialization error: ");
    Serial.println(err.f_str());
    return BAD_REQUEST;  
  }
  JsonObject root = doc.as<JsonObject>();
  if (!root.containsKey("signModel") || !root.containsKey("serialNumber") || !root.containsKey("manufacturingDate")) {
    Serial.println("missing one of the mandatory fields: 'signModel', 'serialNumber', 'manufacturingDate'");
    return BAD_REQUEST;
  }
  String signModel = root["signModel"];
  String serialNumber = root["serialNumber"];
  String manufacturingDate = root["manufacturingDate"];

  if (signModel.isEmpty() || signModel.length() < 5) {
    Serial.println("'signModel' field should have at least 5 characters");
    return BAD_REQUEST;
  }
  if (serialNumber.isEmpty() || serialNumber.length() < 5) {
    Serial.println("'serialNumber' field should have at least 5 characters");
    return BAD_REQUEST;
  }
  if (manufacturingDate.isEmpty() || manufacturingDate.length() < 10) {
    Serial.println("'manufacturingDate' field should have at least 100 characters and preferably in the YYYY/MM/DD format");
    return BAD_REQUEST;
  }

  DeviceInfo newDevInfo;
  strcpy(newDevInfo.signModel, signModel.c_str());
  strcpy(newDevInfo.serialNumber, serialNumber.c_str());
  strcpy(newDevInfo.manufacturingDate, manufacturingDate.c_str());

  persistDeviceInformation(newDevInfo);
  devInfo = newDevInfo;

  return POST_SUCCESS; 
}

void persistDeviceInformation(DeviceInfo& newDevInfo) {
  Serial.println("Persisting config information on EEPROM");
  Serial.println(" signModel: " + String(newDevInfo.signModel));
  Serial.println(" serialNumber: " + String(newDevInfo.serialNumber));
  Serial.println(" manufacturingDate: " + String(newDevInfo.manufacturingDate));

  newDevInfo.setValue = EEPROM_SET;
  EEPROM.begin(STORAGE_SIZE);
  EEPROM.put(DEVICE_INFO_POS, newDevInfo);
  EEPROM.end();

  Serial.println("Done");
}

void loadWifiConfig() {
  Serial.print("Reading Wifi config from EEPROM...");
  EEPROM.begin(STORAGE_SIZE);
  EEPROM.get(WIFI_CONFIG_POS, wifiConfig);
  EEPROM.end();

  if (!wifiConfig.isWifiSet()) { // set default mode to AP
    Serial.println("Failed. Using default");
    setDefaultWifiConfig();
  } else {
    Serial.println("Done");
  }

  Serial.println(" mode: " + wifiConfig.modeAsString());
  Serial.println(" ssid: " + String(wifiConfig.ssid));
  Serial.println(" psk: " + String(wifiConfig.psk));
}

String getWifiConfig() {
  DynamicJsonDocument doc(500);
  if (staConnectionFailed) {
    doc["staConnectionFailed"] = true;
  }
  doc["mode"] = wifiConfig.modeAsString();
  doc["ssid"] = String(wifiConfig.ssid);
  doc["psk"] = String(wifiConfig.psk);

  String message = "";
  serializeJsonPretty(doc, message);
  return message;
}

String getDefaultSSID() {
  // define default ssid using mac address
  String macAddr = WiFi.macAddress();
  String macId = macAddr.substring(macAddr.length()-5); // get last 4 chars of mac
  macId.replace(":", "");
  String ssid = String(AP_SSID) + "_" + macId; // append to default ssid
  return ssid;
}

String getDefaultPsk() {
  return String(AP_PSK);
}

void setDefaultWifiConfig() {
  wifiConfig.setValue = 0;
  wifiConfig.mode = WIFI_AP;

  String ssid = getDefaultSSID();
  memset(wifiConfig.ssid, '\0', sizeof wifiConfig.ssid);
  strcpy(wifiConfig.ssid, ssid.c_str());

  memset(wifiConfig.psk, '\0', sizeof wifiConfig.psk);  
  String psk = getDefaultPsk();
  strcpy(wifiConfig.psk, psk.c_str());
}

PostResult setWifiConfig(String jsonBody) {
  if (jsonBody.isEmpty()) {
    return BAD_REQUEST;
  }
  DynamicJsonDocument doc(250);
  DeserializationError err = deserializeJson(doc, jsonBody);
  if (err) {
    Serial.print("Deserialization error: ");
    Serial.println(err.f_str());
    return BAD_REQUEST;  
  }
  JsonObject root = doc.as<JsonObject>();
  if (!root.containsKey("mode") || !root.containsKey("ssid")) {
    Serial.println("missing 'mode' or 'ssid' fields");
    return BAD_REQUEST;
  }
  String mode = root["mode"];
  String ssid = root["ssid"];
  String psk = root.containsKey("psk") ? root["psk"] : String("");
  return setWifiConfig(mode, ssid, psk);  
}

PostResult setWifiConfig(String mode, String ssid, String psk) {
  bool isAP = mode.equals("ap");
  bool isSTA = mode.equals("sta");
  bool isOFF = mode.equals("off");
  if (!isAP && !isSTA && !isOFF) {
    return BAD_REQUEST;
  }
  if (ssid.isEmpty() || ssid.length() == 0 || ssid.length() > 32) {
    return BAD_REQUEST;
  }
  if (!psk.isEmpty() && (psk.length() > 64 || psk.length() < 8)) {
    return BAD_REQUEST;
  }

  WifiConfig newWifiConfig;
  newWifiConfig.mode = isAP ?  WIFI_AP :
                    isSTA ? WIFI_STA : WIFI_OFF;
  strcpy(newWifiConfig.ssid, ssid.c_str());
  strcpy(newWifiConfig.psk, psk.c_str());

  persistWifiConfig(newWifiConfig);
  wifiConfig = newWifiConfig;

  return POST_SUCCESS;
}

void persistWifiConfig(WifiConfig& newWifiConfig) {
  Serial.println("Persisting config information on EEPROM...");
  Serial.println(" mode: " + newWifiConfig.modeAsString());
  Serial.println(" ssid: " + String(newWifiConfig.ssid));
  Serial.println(" psk: " + String(newWifiConfig.psk));

  newWifiConfig.setValue = EEPROM_SET;
  EEPROM.begin(STORAGE_SIZE);
  EEPROM.put(WIFI_CONFIG_POS, newWifiConfig);
  EEPROM.end();

  Serial.println("Done");
}

void loadUserSettings() {
  Serial.print("Reading user Settings from EEPROM...");
  EEPROM.begin(STORAGE_SIZE);
  EEPROM.get(SETTINGS_POS, settings);
  EEPROM.end();

  if (!settings.isSet()) {
    Serial.println("Failed. Using default");
    settings.setValue = 0;
    settings.tzOffsetInMinutes = BR_TIMEZONE;
    settings.autoTurnOffPeriod = 30;
    settings.autoSleep = { true, 15, 8, 20, WEEKDAY };
  } else {
    // validate fields
    settings.tzOffsetInMinutes = std::min(std::max(settings.tzOffsetInMinutes, MIN_TZ_OFFSET), MAX_TZ_OFFSET);
    settings.autoTurnOffPeriod = std::min(settings.autoTurnOffPeriod, MAX_AUTO_TURNOFF_PERIOD);
    settings.autoSleep.activeHourStart %= 24;
    settings.autoSleep.activeHourEnd %= 24;
    Serial.println("Done");
  }

  Serial.println(" tz-offset: " + String(settings.tzOffsetInMinutes) + "min");
  Serial.println(" auto-turnoff period: " + String(settings.autoTurnOffPeriod) + "min");
  Serial.println(" auto-sleep: enabled=" + String(settings.autoSleep.enabled ? "true" : "false") + ", period=" + String(settings.autoSleep.period) + "min");
  Serial.println(" auto-sleep active time: start=" + String(settings.autoSleep.activeHourStart) + "h, end=" + String(settings.autoSleep.activeHourEnd) + "h, days=" + dayOfWeekToString(settings.autoSleep.activeDaysOfWeek));
}

String getUserSettings() {
  DynamicJsonDocument doc(500);
  doc["tzOffset"] = settings.tzOffsetInMinutes;
  doc["autoTurnOffPeriod"] = settings.autoTurnOffPeriod;
  DynamicJsonDocument autoSleep(300); 
  autoSleep["enabled"] = settings.autoSleep.enabled;
  autoSleep["period"] = settings.autoSleep.period;
  autoSleep["activeHourStart"] = settings.autoSleep.activeHourStart;
  autoSleep["activeHourEnd"] = settings.autoSleep.activeHourEnd;
  JsonArray days = autoSleep.createNestedArray("activeDaysOfWeek");
  for (auto& d : settings.autoSleep.getActiveDaysOfWeek()) {
    String dText = singleDayOfWeekToString(d);
    dText.toLowerCase();
    days.add(dText);
  }
  doc["autoSleep"] = autoSleep;

  String message = "";
  serializeJsonPretty(doc, message);
  return message;
}

int getTzOffsetInSecods() {
  return settings.tzOffsetInMinutes * 60; // in seconds
}

long getAutoTurnOffPeriod() {
  return settings.autoTurnOffPeriod * 60; // in seconds
}

PostResult setUserSettings(String jsonBody) {
  if (jsonBody.isEmpty()) {
    return BAD_REQUEST;
  }
  DynamicJsonDocument doc(500);
  DeserializationError err = deserializeJson(doc, jsonBody);
  if (err) {
    Serial.print("Deserialization error: ");
    Serial.println(err.f_str());
    return BAD_REQUEST;  
  }
  JsonObject root = doc.as<JsonObject>();
  if (!root.containsKey("autoSleep") && !root.containsKey("autoTurnOffPeriod") && !root.containsKey("tzOffset")) {
    Serial.println("missing fields: 'autoSleep', 'autoTurnOffPeriod' and 'tzOffset'");
    return BAD_REQUEST;
  }
  
  Settings newSettings = settings;

  if (root.containsKey("tzOffset")) {
    newSettings.tzOffsetInMinutes = std::min(std::max(root["tzOffset"].as<int>(), MIN_TZ_OFFSET), MAX_TZ_OFFSET);
  }
  if (root.containsKey("autoTurnOffPeriod")) {
    newSettings.autoTurnOffPeriod = std::min(root["autoTurnOffPeriod"].as<ushort>(), MAX_AUTO_TURNOFF_PERIOD);
  }
  if (root.containsKey("autoSleep")) {
    JsonObject autoSleep = root["autoSleep"].as<JsonObject>();
    if (!autoSleep.containsKey("enabled")) {
      Serial.println("missing the 'enabled' field for autoSleep");
      return BAD_REQUEST;
    }
    newSettings.autoSleep.enabled = autoSleep["enabled"].as<bool>();

    if (newSettings.autoSleep.enabled) {
      if (autoSleep.containsKey("period") && autoSleep.containsKey("activeHourStart") && autoSleep.containsKey("activeHourEnd")) {
        newSettings.autoSleep.period = autoSleep["period"].as<ushort>();
        newSettings.autoSleep.activeHourStart = autoSleep["activeHourStart"].as<byte>() % 24;
        newSettings.autoSleep.activeHourEnd = autoSleep["activeHourEnd"].as<byte>() % 24;
        if (autoSleep.containsKey("activeDaysOfWeek")) {
          std::vector<DayOfWeek> daysOfWeek;
          for (JsonVariant day : autoSleep["activeDaysOfWeek"].as<JsonArray>()) {
            String d = String(day.as<const char*>());
            daysOfWeek.push_back(stringToDayOfWeek(d));
          }
          newSettings.autoSleep.setActiveDaysOfWeek(daysOfWeek);
        }
      } else {
        Serial.println("missing any of the autoSleep fields: 'period', 'activeHourStart', and 'activeHourEnd'");
        return BAD_REQUEST;
      }
    } 
  }

  // persist on EEPROM
  persistUserSettings(newSettings);
  settings = newSettings;
  setTzOffset(settings.tzOffsetInMinutes);

  return POST_SUCCESS;  
}

void persistUserSettings(Settings& conf) {
  // persist on EEPROM
  Serial.println("Persisting user settings on EEPROM");
  Serial.println(" tz-offset: " + String(conf.tzOffsetInMinutes) + "min");
  Serial.println(" auto-turnoff period: " + String(conf.autoTurnOffPeriod) + "min");
  Serial.println(" auto-sleep: enabled=" + String(conf.autoSleep.enabled ? "true" : "false") + ", period=" + String(conf.autoSleep.period) + "min");
  Serial.println(" auto-sleep active time: start=" + String(conf.autoSleep.activeHourStart) + "h, end=" + String(conf.autoSleep.activeHourEnd) + "h, days=" + dayOfWeekToString(conf.autoSleep.activeDaysOfWeek));

  conf.setValue = EEPROM_SET;
  EEPROM.begin(STORAGE_SIZE);
  EEPROM.put(SETTINGS_POS, conf);
  EEPROM.end();

  Serial.println("Done");
}

void factoryReset(bool fullReset) {
  Serial.print("Clearing Wifi config, user settings" + String(fullReset ? ", and device information" : "") + " on EEPROM...");
  EEPROM.begin(STORAGE_SIZE);
  for (int i=0; i < STORAGE_BLOCK; i++) {
    // reset device information
    if (fullReset) EEPROM.write(DEVICE_INFO_POS + i, 255);
    // reset wifi config
    EEPROM.write(WIFI_CONFIG_POS + i, 255);
    // reset user settings
    EEPROM.write(SETTINGS_POS + i, 255);
  }
  EEPROM.end();
  Serial.println("Done");
}
