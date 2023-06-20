#include <map>
#include <array>

#define ANODE_CONTROL_LED true
#define CATHODE_CONTROL_LED false

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
  { "busy",           { {WHITE,    {D1,CATHODE_CONTROL_LED}}, 
                        {RED,      {D2,CATHODE_CONTROL_LED}} }},
  { "do-not-disturb", { {RED,      {D3,CATHODE_CONTROL_LED}} }},
  { "camera",         { {YELLOW,   {D5,CATHODE_CONTROL_LED}} }},
  { "microphone",     { {YELLOW,   {D6,CATHODE_CONTROL_LED}} }},
  { "alert",          { {RGB_RED,  {D7,ANODE_CONTROL_LED}}, 
                        {RGB_GREEN,{D8,ANODE_CONTROL_LED}},
                        {RGB_BLUE, {D9,ANODE_CONTROL_LED}} }}
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
  ulong ttl;
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

