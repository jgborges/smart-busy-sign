#ifndef SIGN_H
#define SIGN_H

#include "defines.h"

struct PanelStatus {
  String name;
  PanelState state;
  String color;
  byte intensity;
  ushort ttl;
};

void buildStatusMap(const PanelSetupMap& panelSetup);
void applySignStatus();
void applySignStatus(PanelStatus panel);
String getSignStatus();
ParsingResult setSignStatus(String statusJson);
bool isLedRGB(LedTypes led);
bool isPanelOn(PanelState state);
String panelStateToString(PanelState state);
PanelState parsePanelState(String state);

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

#endif