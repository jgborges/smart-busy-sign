#include <map>
#include <array>
#include <vector>

#define DEBUG_FAUXMO Serial

#define ANODE_CONTROL_LED true
#define CATHODE_CONTROL_LED false

#define BLINK_CHECK_PERIOD_MS 100   // every 100 ms
#define TTL_CHECK_PERIOD_MS   5000  // every 5 seconds
#define SLEEP_CHECK_PERIOD_MS 60000 // every 1 minute

#define EEPROM_SET 42 // magic number to differentiate random bits from actual values written in EEPROM

enum LedTypes {
  WHITE, YELLOW, RED, GREEN, BLUE, RGB_RED, RGB_GREEN, RGB_BLUE 
};

struct LedSetup {
  byte gpio;
  bool isPosAnode;
};

typedef std::map<String, std::map<LedTypes, LedSetup>> PanelSetup;

// initialize with default panel setup
const PanelSetup panelSetupMap = {
  { "busy",           { {WHITE,    {D5,CATHODE_CONTROL_LED}} }}, 
  //                    {RED,      {D3,CATHODE_CONTROL_LED}} }},
  { "camera",         { {YELLOW,   {D6,CATHODE_CONTROL_LED}} }},
  { "microphone",     { {YELLOW,   {D7,CATHODE_CONTROL_LED}} }},
  { "do-not-disturb", { {RED,      {D8,CATHODE_CONTROL_LED}} }}
  //{ "alert",        { {RGB_RED,  {D7,ANODE_CONTROL_LED}}, 
  //                    {RGB_GREEN,{D8,ANODE_CONTROL_LED}},
  //                    {RGB_BLUE, {D9,ANODE_CONTROL_LED}} }}
};

enum BlinkingType { BLINKING_NORMAL, BLINKING_FAST, BLINKING_SLOW, BLINKING_OFF };

const int NumBlinkingTypes = BLINKING_OFF - BLINKING_NORMAL;

enum PanelState {
  PANEL_DISABLED,
  PANEL_OFF,
  PANEL_ON,
  PANEL_BLINKING,
  PANEL_BLINKING_FAST,
  PANEL_BLINKING_SLOW
};

struct PanelStatus {
  String name;
  PanelState state;
  String color;
  byte intensity;
  ushort ttl;
};

struct GpioState {
  PanelState state;
  byte intensity;
  bool isPosAnode;

  BlinkingType blinkingType() {
    switch (this->state) {
      case PANEL_BLINKING:
        return BLINKING_NORMAL;
      case PANEL_BLINKING_FAST:
        return BLINKING_FAST;
      case PANEL_BLINKING_SLOW:
        return BLINKING_SLOW;
      default:
        return BLINKING_OFF;
    }
  }
};

struct BlinkingState {
  // blinking params
  ushort intervalOn;
  ushort intervalOff;
  // manage on/off state for blinking effect
  bool isLightOn;
  ulong lastStateChangeMillis;
};

enum ParsingResult { 
  PARSE_SUCCESS, 
  NO_PANEL_FOUND,
  JSON_PARSING_ERROR, 
  JSON_MISSING_PANELS_FIELD, 
  JSON_MISSING_NAME_FIELD, 
  JSON_MISSING_STATE_FIELD,
};

enum PostResult { 
  POST_SUCCESS, 
  BAD_REQUEST,
  SERVER_ERROR
};

// permanent sign information
struct DeviceInfo {
  char signModel[36];
  char serialNumber[26];
  char manufacturingDate[26];
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
    }else if (this->mode == WIFI_OFF) {
      return "off";
    } else {
      return "unknown";
    }
  }
};

// User settings
enum DayOfWeek : byte { 
  NONE=0,
  SUNDAY=1, MONDAY=2, TUESDAY=4, WEDNESDAY=8, THURSDAY=16, FRIDAY=32, SATURDAY=64, 
  WEEKDAY=MONDAY|TUESDAY|WEDNESDAY|THURSDAY|FRIDAY, 
  WEEKEND=SATURDAY|SUNDAY, 
  ALL_DAYS=SUNDAY|MONDAY|TUESDAY|WEDNESDAY|THURSDAY|FRIDAY|SATURDAY,
};

const std::array AllDaysOfWeek = { SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY };

struct AutoSleepSettings {
  bool enabled;
  ushort period;
  byte activeHourStart;
  byte activeHourEnd;
  DayOfWeek activeDaysOfWeek; // coded as bits in a byte

  std::vector<DayOfWeek> getActiveDaysOfWeek() {
    std::vector<DayOfWeek> out;
    
    for (int i=0; i < AllDaysOfWeek.size(); i++) {
      if ((this->activeDaysOfWeek & AllDaysOfWeek[i]) > 0) {
        out.push_back(AllDaysOfWeek[i]);
      }
    }
    return out;
  }

  void setActiveDaysOfWeek(std::vector<DayOfWeek> days) {
    byte newValue = 0;
    for(int i = 0; i < days.size(); i++) {
      DayOfWeek d = days[i];
      if (d == NONE || d == WEEKDAY || d == WEEKEND || d == ALL_DAYS) {
        continue;
      }
      newValue = (byte)(newValue | (byte)d);
    }
    this->activeDaysOfWeek = (DayOfWeek)newValue;
  }
};

struct Settings {
  int tzOffsetInMinutes;
  ushort autoTurnOffPeriod;
  AutoSleepSettings autoSleep;
  byte setValue; // make it last field so any change will invalidate the whole struct

  bool isSet() {
    return this->setValue == EEPROM_SET;
  }
};
