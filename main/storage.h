#ifndef STORAGE_H
#define STORAGE_H

#include "defines.h"

#ifndef AP_SSID
#define AP_SSID "Smart-Busy-Sign"
#define AP_PSK "smartsign"
#endif

#define BR_TIMEZONE -3*60
#define MAX_TZ_OFFSET 11*60
#define MIN_TZ_OFFSET -11*60
#define MAX_AUTO_TURNOFF_PERIOD (ushort)(24*60)

// permanent sign information

struct PanelInfo {
  bool enabled;
  char name[25];
  LedTypes led;
  LedSetup pin;
};

struct DeviceInfo {
  char signModel[36];
  char serialNumber[26];
  char manufacturingDate[26];
  PanelInfo panels[MAX_PINS];
  byte setValue; // make it last field so any change will invalidate the whole struct

  bool isSet() {
    return this->setValue == EEPROM_SET;
  }
};

// wifi connection information

struct WifiConfig {
  WiFiMode_t mode;
  char ssid[33]; // ssd can be 32 char at most
  char psk[63];  // psk can be 62 char at most
  byte setValue; // make it last field so any change will invalidate the whole struct

  bool isWifiSet() {
    return this->setValue == EEPROM_SET;
  }

  String modeAsString() {
    if (this->mode == WIFI_STA) {
      return "sta";
    } else if (this->mode == WIFI_AP) {
      return "ap";
    } else if (this->mode == WIFI_OFF) {
      return "off";
    } else {
      return "unknown";
    }
  }
};


void loadDeviceInformation();
String getDeviceInformation();
PostResult setDeviceInformation(String jsonBody);
void persistDeviceInformation(DeviceInfo& newDevInfo);

void loadWifiConfig();
String getWifiConfig();
bool getWifiConfig(WiFiMode_t& mode, String& ssid, String& psk);
PostResult setWifiConfig(String jsonBody);
PostResult setWifiConfig(String mode, String ssid, String psk);
void persistWifiConfig(WifiConfig& newWifiConfig);

void loadUserSettings();
String getUserSettings();
PostResult setUserSettings(String jsonBody);
void persistUserSettings(Settings& conf);

#endif