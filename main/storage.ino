#include <EEPROM.h>

#define EEPROM_SET 42 // magig number to differentiate random bits from actual values written in EEPROM

struct Storage {
  // permanent sign information
  byte setValue;
  char signModel[36];
  char serialNumber[26];
  char manufacturingDate[26];

  // wifi connection information
  byte wifiSetValue;
  char wifi_ssid[33]; // ssd can be 32 char at most
  char wifi_pwd[63];  // password can be 62 char at most

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
    strncpy(storage.signModel, SIGN_MODEL, sizeof(storage.signModel));
    //0, SIGN_MODEL, SIGN_SN, "2023-06-01",
    //0, "", "",
  }
}
