#include <EEPROM.h>

#define EEPROM_SET 42 // magig number to differentiate random bits from actual values written in EEPROM

enum WifiMode { AP, CLIENT };

struct Storage {
  // permanent sign information
  byte setValue;
  char signModel[36];
  char serialNumber[26];
  char manufacturingDate[26];

  // wifi connection information
  byte wifiSetValue;
  WifiMode wifiMode;
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
    //memcpy(&storage, &tmp, sizeof(Storage));
    storage = tmp;
  } else {
    // load default values
    storage.setValue = 0;
    strncpy(storage.signModel, "test 1", sizeof(storage.signModel));

    storage.wifiSetValue = 0;
    storage.wifiMode = AP;
    strncpy(storage.wifiSsid, "Smart Busy Sign", sizeof(storage.wifiSsid));
    strncpy(storage.wifiPwd, "abc123456", sizeof(storage.wifiPwd));
    
    //0, SIGN_MODEL, SIGN_SN, "2023-06-01",
  }
}

String getWifiStorage() {
  String mode = storage.wifiMode == AP ? "AP" : "Client";
  String msg = "{ ";
  msg += "\"mode\": \"" + mode + "\",";
  msg += "\"ssid\": \"" + String(storage.wifiSsid) + "\", ";
  msg += "\"password\": \"" + String(storage.wifiPwd) + "\" ";
  msg += "}";
  return msg;
}
