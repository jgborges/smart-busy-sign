#include <EEPROM.h>

#ifndef AP_SSID
#define AP_SSID "Smart-Busy-Sign"
#define AP_PSK "smartsign"
#endif

#define EEPROM_SET 42 // magig number to differentiate random bits from actual values written in EEPROM

struct DeviceInfo {
  // permanent sign information
  byte setValue;
  char signModel[36];
  char serialNumber[26];
  char manufacturingDate[26];

  bool isSet() {
    return this->setValue == EEPROM_SET;
  }
};

struct WifiConfig {
  // wifi connection information
  byte setValue;
  WiFiMode_t mode;
  char ssid[33]; // ssd can be 32 char at most
  char psk[63];  // password can be 62 char at most

  bool isWifiSet() {
    return this->setValue == EEPROM_SET;
  }

  String modeAsString() {
    if (this->mode == WIFI_STA) {
      return "sta";
    } else if (this->mode == WIFI_AP) {
      return "ap";
    }else if (this->mode == WIFI_OFF) {
      return "off";
    } else {
      return "unknown";
    }
  }
};

DeviceInfo devInfo = {};
WifiConfig wifiConfig = {};

#define STORAGE_SIZE    4096.
#define STORAGE_BLOCK   512
#define DEVICE_INFO_POS 0*STORAGE_BLOCK
#define WIFI_CONFIG_POS 1*STORAGE_BLOCK

void setupStorage() {
  readDeviceInformation();
  readWifiConfig();
}

void readDeviceInformation() {
  Serial.print("Reading device information from EEPROM...");
  EEPROM.begin(STORAGE_SIZE);
  EEPROM.get(DEVICE_INFO_POS, devInfo);
  EEPROM.end();

  if (!devInfo.isSet()) {
    Serial.println("Failed. Using default");
    devInfo.setValue = 0;
    strncpy(devInfo.signModel, "smart-busy-sign-v1", sizeof(devInfo.signModel));
    strncpy(devInfo.serialNumber, "0000000001", sizeof(devInfo.serialNumber));
    strncpy(devInfo.manufacturingDate, "2023-06-01", sizeof(devInfo.manufacturingDate));
  } else {
    Serial.println("Done");
  }

  Serial.println(" sign-model: " + String(devInfo.signModel));
  Serial.println(" serial-number: " + String(devInfo.serialNumber));
  Serial.println(" manufacturing-date: " + String(devInfo.manufacturingDate));
}

void setDeviceInformation() {
}

void readWifiConfig() {
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

String getDefaultSSID() {
  // define default ssid using mac address
  String macAddr = WiFi.macAddress();
  String macId = macAddr.substring(macAddr.length()-5); // get last 4 chars of mac
  macId.replace(":", "");
  String ssid = String(AP_SSID) + "_" + macId; // append to default ssid
  return ssid;
}

String getDefaultPassword() {
  return String(AP_PSK);
}

String getWifiConfig() {
  String msg = "{ ";
  if (staConnectionFailed) {
    msg += "\"staConnectionFailed\": true,";
  }
  msg += "\"mode\": \"" + wifiConfig.modeAsString() + "\",";
  msg += "\"ssid\": \"" + String(wifiConfig.ssid) + "\", ";
  msg += "\"password\": \"" + String(wifiConfig.psk) + "\" ";
  msg += "}";
  return msg;
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
    Serial.println("missing mode or ssid fields");
    return BAD_REQUEST;
  }
  String mode = root["mode"];
  String ssid = root["ssid"];
  String password = root.containsKey("password") ? root["password"] : String("");
  return setWifiConfig(mode, ssid, password);  
}

PostResult setWifiConfig(String mode, String ssid, String password) {
  bool isAP = mode.equals("ap");
  bool isSTA = mode.equals("sta");
  bool isOFF = mode.equals("off");
  if (!isAP && !isSTA && !isOFF) {
    return BAD_REQUEST;
  }
  if (ssid.isEmpty() || ssid.length() == 0 || ssid.length() > 32) {
    return BAD_REQUEST;
  }
  if (!password.isEmpty() && (password.length() > 64 || password.length() < 8)) {
    return BAD_REQUEST;
  }

  wifiConfig.setValue = EEPROM_SET;
  wifiConfig.mode = isAP ?  WIFI_AP :
                    isSTA ? WIFI_STA : WIFI_OFF;
  strcpy(wifiConfig.ssid, ssid.c_str());
  strcpy(wifiConfig.psk, password.c_str());

  Serial.println("Persisting config information on EEPROM");

  Serial.println("Content to write:");
  Serial.println(" mode: " + wifiConfig.modeAsString());
  Serial.println(" ssid: " + String(wifiConfig.ssid));
  Serial.println(" psk: " + String(wifiConfig.psk));

  EEPROM.begin(STORAGE_SIZE);
  EEPROM.put(WIFI_CONFIG_POS, wifiConfig);
  EEPROM.end();

  return POST_SUCCESS;
}

void setDefaultWifiConfig() {
  wifiConfig.setValue = 0;
  wifiConfig.mode = WIFI_AP;

  String ssid = getDefaultSSID();
  memset(wifiConfig.ssid, '\0', sizeof wifiConfig.ssid);
  strcpy(wifiConfig.ssid, ssid.c_str());

  memset(wifiConfig.psk, '\0', sizeof wifiConfig.psk);  
  String password = getDefaultPassword();
  strcpy(wifiConfig.psk, password.c_str());
}

void factoryReset(bool fullReset) {
  Serial.print("Clearing Wifi config " + String(fullReset ? "and device information " : "") + "on EEPROM...");
  EEPROM.begin(STORAGE_SIZE);
  for (int i=0; i < STORAGE_BLOCK; i++) {
    // reset device information
    if (fullReset) EEPROM.write(DEVICE_INFO_POS + i, 255);
    // reset wifi config
    EEPROM.write(WIFI_CONFIG_POS + i, 255);
  }
  EEPROM.end();
  Serial.println("Done");
}
