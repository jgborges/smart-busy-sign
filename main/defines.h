#ifndef DEFINES_H
#define DEFINES_H

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
  #include <ESP8266NetBIOS.h>

  #define BOARD_MODEL ARDUINO_BOARD
  #define BOARD_ID ARDUINO_BOARD_ID
  #define BOARD_VERSION ARDUINO_ESP8266_RELEASE
  #define MAX_PINS NUM_DIGITAL_PINS
  
  ADC_MODE(ADC_VCC); // measure voltage in ADC input
#elif defined(ESP32)
  #include <WiFi.h>
  #include <ESPmDNS.h>
  #include <NetBIOS.h>
  #include "driver/gpio.h"

  #define BOARD_MODEL ARDUINO_BOARD
  #define BOARD_ID ARDUINO_VARIANT
  #define BOARD_VERSION "ESP32-WROOM-32D"
  #define MAX_PINS NUM_DIGITAL_PINS

  #define D0 A0
  #define D1 A3
  #define D2 A4
  #define D3 A5
  #define D4 A6
  #define D5 A7
  #define D6 A10
  #define D7 A11
  #define D8 A12
  #define D9 A13
  #define D10 A14
  #define D11 A15
  #define D12 A16
  #define D13 A17
  #define D14 A18
  #define D15 A19

  #define LED_BUILTIN A12
#else
#error "This is not a ESP8266 or ESP32 board!"
#endif

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

typedef std::map<LedTypes, LedSetup> LedSetupMap;
typedef std::map<String, LedSetupMap> PanelSetupMap;

enum PanelState {
  PANEL_DISABLED,
  PANEL_OFF,
  PANEL_ON
};

enum BlinkingType { BLINKING_NORMAL, BLINKING_FAST, BLINKING_SLOW, BLINKING_OFF };

const int NumBlinkingTypes = BLINKING_OFF - BLINKING_NORMAL;

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

// User settings
enum DayOfWeek : byte { 
  NONE=0,
  SUNDAY=1, MONDAY=2, TUESDAY=4, WEDNESDAY=8, THURSDAY=16, FRIDAY=32, SATURDAY=64, 
  WEEKDAY=MONDAY|TUESDAY|WEDNESDAY|THURSDAY|FRIDAY, 
  WEEKEND=SATURDAY|SUNDAY, 
  ALL_DAYS=SUNDAY|MONDAY|TUESDAY|WEDNESDAY|THURSDAY|FRIDAY|SATURDAY,
};

//const std::array<DayOfWeek, 7> AllDaysOfWeek = { SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY };
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

#endif
