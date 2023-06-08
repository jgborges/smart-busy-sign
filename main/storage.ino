#include <EEPROM.h>

#ifndef AP_SSID
#define AP_SSID "Smart-Busy-Sign"
#define AP_PSK "smartsign"
#endif

#define EEPROM_SET 42 // magig number to differentiate random bits from actual values written in EEPROM

struct Storage {
  // permanent sign information
  byte setValue;
  char signModel[36];
  char serialNumber[26];
  char manufacturingDate[26];

  // wifi connection information
  byte wifiSetValue;
  WiFiMode_t wifiMode;
  char wifiSsid[33]; // ssd can be 32 char at most
  char wifiPwd[63];  // password can be 62 char at most

  bool isSet() {
    return this->setValue == EEPROM_SET;
  }

  bool isWifiSet() {
    return this->wifiSetValue == EEPROM_SET;
  }
};

Storage storage = {};

void setupStorage() {
  Storage tmp = {};
  EEPROM.get(0, tmp);

  if (tmp.isSet()) {
    Serial.println("Storage information fetched from EEPROM");
    memcpy(&storage, &tmp, sizeof(Storage));
    //storage = tmp;
  } else {
    // load default values
    storage.setValue = 0;
    strncpy(storage.signModel, "smart-busy-sign-v1", sizeof(storage.signModel));
    strncpy(storage.serialNumber, "0000000001", sizeof(storage.serialNumber));
    strncpy(storage.manufacturingDate, "2023-06-01", sizeof(storage.manufacturingDate));

    storage.wifiSetValue = 0;
    memset(storage.wifiSsid, '\0', sizeof storage.wifiSsid);
    memset(storage.wifiPwd, '\0', sizeof storage.wifiPwd);

    // set default mode to AP
    storage.wifiMode = getDefaultMode();
    
    String ssid = getDefaultSSID();
    strcpy(storage.wifiSsid, ssid.c_str());

    String password = getDefaultPassword();
    strcpy(storage.wifiPwd, password.c_str());
  }
}

WiFiMode_t getDefaultMode() {
  return WIFI_AP;
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

String getWifiStorage() {
  String mode = storage.wifiMode == WIFI_AP ? "ap" : 
                storage.wifiMode == WIFI_STA ? "sta" : 
                storage.wifiMode == WIFI_OFF ? "off" : "unknown";
  String msg = "{ ";
  msg += "\"mode\": \"" + mode + "\",";
  msg += "\"ssid\": \"" + String(storage.wifiSsid) + "\", ";
  msg += "\"password\": \"" + String(storage.wifiPwd) + "\" ";
  msg += "}";
  return msg;
}

PostResult setWifiStorage(String mode, String ssid, String password) {
  bool isAP = mode.equals("ap");
  bool isSTA = mode.equals("sta");
  bool isOFF = mode.equals("off");
  if (!isAP && !isSTA && !isOFF) {
    return BAD_REQUEST;
  }
  if (ssid.isEmpty() || ssid.length() == 0 || ssid.length() > 32) {
    return BAD_REQUEST;
  }
  if (password.length() > 0 && (password.length() > 64 || password.length() < 8)) {
    return BAD_REQUEST;
  }

  storage.wifiSetValue = EEPROM_SET;
  storage.wifiMode = isAP ?  WIFI_AP :
                     isSTA ? WIFI_STA : WIFI_OFF;
  strcpy(storage.wifiSsid, ssid.c_str());
  strcpy(storage.wifiPwd, password.c_str());

  Serial.println("Persisting storage information on EEPROM");
  EEPROM.put(0, storage);

  return POST_SUCCESS;
}
