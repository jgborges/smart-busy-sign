#ifndef SIGN_H
#define SIGN_H

void buildStatusMap(const PanelSetupMap& panelSetup);
void applySignStatus();
void applySignStatus(PanelStatus panel);
String getSignStatus();
ParsingResult setSignStatus(String statusJson);
bool isLedRGB(LedTypes led);
bool isPanelOn(PanelState state);

const std::map<LedTypes, String> LedToColor = {
  { WHITE, "white" },
  { YELLOW, "yellow" },
  { RED, "red" },
  { GREEN, "green" },
  { BLUE, "blue" },
  { RGB_RED, "#FF0000" },
  { RGB_GREEN, "#FF0000" },
  { RGB_BLUE, "#FF0000" },
};

const std::map<String, LedTypes> ColorToLed = {
  { "white", WHITE },
  { "yellow", YELLOW },
  { "red", RED },
  { "green", GREEN },
  { "blue", BLUE },
};

// initialize with default panel setup
const PanelSetupMap panelSetupMap = {
  { "busy",           { {WHITE,    {D3,CATHODE_CONTROL_LED}} }}, 
  //                    {RED,      {D3,CATHODE_CONTROL_LED}} }},
  { "camera",         { {YELLOW,   {D5,CATHODE_CONTROL_LED}} }},
  { "microphone",     { {YELLOW,   {D6,CATHODE_CONTROL_LED}} }},
  { "do-not-disturb", { {RED,      {D7,CATHODE_CONTROL_LED}} }}
  //{ "alert",        { {RGB_RED,  {D7,ANODE_CONTROL_LED}}, 
  //                    {RGB_GREEN,{D8,ANODE_CONTROL_LED}},
  //                    {RGB_BLUE, {D9,ANODE_CONTROL_LED}} }}
};

String panelStateToString(PanelState state) {
  switch (state) {
    default:
    case PANEL_DISABLED:
      return "disabled";
    case PANEL_OFF:
      return "off";
    case PANEL_ON:
      return "on-solid";
    case PANEL_BLINKING:
      return "on-blinking";
    case PANEL_BLINKING_FAST:
      return "on-blinking-fast";
    case PANEL_BLINKING_SLOW:
      return "on-blinking-slow";
  }
}

PanelState parsePanelState(String state) {
  if (state.equals("off")) {
    return PANEL_OFF;
  } else if (state.equals("on-solid")) {
    return PANEL_ON;
  } else if (state.equals("on-blinking")) {
    return PANEL_BLINKING;
  } else if (state.equals("on-blinking-fast")) {
    return PANEL_BLINKING_FAST;
  } else if (state.equals("on-blinking-slow")) {
    return PANEL_BLINKING_SLOW;
  } else {
    return PANEL_DISABLED;
  }
}

#endif