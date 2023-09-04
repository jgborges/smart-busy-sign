/*
  A sign is defined by a list of panels, which are defined by a light color (red, white, blue, etc), 
  state (on, off), pattern (blinking), and intensty (0 - 255).

  The current state of the sign, called status, can be defined by the following structure

  {
    "model": "smart-sign-1",
    "serial_number": "123456789",
    "hw-model": "ESP8266 NodeMCU",
    "hw-version": "3.1.2",
    "server-version": "1.0",
    "assemble-date": "2023-06-01",
    "panels": [
      {
        "name": "busy",
        "color": "white",
        "intensity": 50,
        "state": "off",
      },
      {
        "name": "do-not-disturb",
        "color": "red",
        "intensity": 255,
        "state": "on-solid",
      },
      {
        "name": "camera",
        "color": "yellow",
        "intensity": 255,
        "state": "on-blinking",
      },
      {
        "name": "microphone",
        "color": "yellow",
        "intensity": 255,
        "state": "off",
      }
    ]
  } 

*/

#include "sign.h"

#define SIGN_SN_DEFAULT 10001

#define SIGN_MODEL "smart-busy-sign_V0"
#define SIGN_VERSION "V0"

#define RESET_GPIO D3
#define RESET_GPIO_MODE INPUT_PULLUP
#define RESET_GPIO_PULLUP true

const uint SHORT_PRESS_TIME = 300;
const uint LONG_PRESS_TIME = 1500;
const uint HARD_RESET_TIME = 10000;

// initialize with default panel setup
const PanelSetupMap panelSetupMap = {
  { "busy",           { {WHITE,    {D1,ANODE_CONTROL_LED}} }}, 
  //                    {RED,      {D3,CATHODE_CONTROL_LED}} }},
  { "camera",         { {YELLOW,   {D2,ANODE_CONTROL_LED}} }},
  { "microphone",     { {YELLOW,   {D5,ANODE_CONTROL_LED}} }},
  { "do-not-disturb", { {RED,      {D6,ANODE_CONTROL_LED}} }}
  //{ "alert",        { {RGB_RED,  {D7,ANODE_CONTROL_LED}}, 
  //                    {RGB_GREEN,{D8,ANODE_CONTROL_LED}},
  //                    {RGB_BLUE, {D9,ANODE_CONTROL_LED}} }}
};

std::map<String, PanelStatus> statusMap;

ulong lastTTLCheck;

void setupSign() {
  //buildPanelSetupMap(devInfo, panelSetupMap);
  setupGpio(panelSetupMap); // set all to disabled

  buildStatusMap(panelSetupMap);
  // appy default sign status
  applySignStatus(); // turn all off as gpios are initiated in DISABLED state

  // setup button interface
  pinMode(RESET_GPIO, RESET_GPIO_MODE);

  lastTTLCheck = millis();
}

// void buildPanelSetupMap(const DeviceInfo& devInfo, PanelSetupMap& panelSetup) {
//   panelSetup.clear();
//   for (byte gpio=0; gpio < MAX_PINS; gpio++) {
//     const PanelInfo& panel = devInfo.panels[gpio];
//     if (!panel.enabled) {
//       continue;
//     }
//     String name = String(panel.name);
//     if (panelSetup.count(name)) {
//       auto ledSetupMap = panelSetup.at(name);
//       if (!ledSetupMap.count(panel.led)) {
//         ledSetupMap.insert({ panel.led, panel.pin });
//       }
//     } else {
//       panelSetup.insert({ name, {{ panel.led, panel.pin }} });
//     }
//   }
// }

void buildStatusMap(const PanelSetupMap& panelSetup) {
  statusMap.clear();
  for (auto& item: panelSetup) {
    String name = item.first;
    LedTypes firstLed = item.second.begin()->first;
    String color = LedToColor.at(firstLed);
    if (isLedRGB(firstLed)) {
      color = "#FFFFFF"; // equivalent to white
    }
    statusMap.insert(std::pair<String, PanelStatus>(name, { name, PANEL_OFF, BLINKING_OFF, color, 255 }));
  }
}

void getDeviceMap(std::map<String, bool>& deviceMap) {
  deviceMap.clear();
  for (auto& item: panelSetupMap) {
    String name = item.first;
    LedTypes firstLed = item.second.begin()->first;
    deviceMap.insert({ name, isLedRGB(firstLed) });
  }
}

void handleButton() {
  ulong pressPeriod = checkButtonPressed(LONG_PRESS_TIME+100);
  if (pressPeriod >= LONG_PRESS_TIME) {
    Serial.println("Long press. Turning all panels off...");
    turnAllPanelsOff();
    delay(300);
  } else if (pressPeriod >= SHORT_PRESS_TIME) {
    Serial.println("Short press. Turning next panel on...");
    turnNextPanelOn();
    delay(300);
  }
}

bool checkHardReset() {
  ulong pressPeriod = checkButtonPressed(HARD_RESET_TIME);
  return pressPeriod >= HARD_RESET_TIME;
}

ulong checkButtonPressed(int timeout) {
  const int reference = RESET_GPIO_PULLUP ? LOW : HIGH;
  ulong start = millis();
  ulong elapsed = 0;
  while (digitalRead(RESET_GPIO) == reference) {
    elapsed = millis() - start;
    if (elapsed >= timeout) {
      break;
    }
    delay(100);
  }
  if (elapsed > 0) {
    Serial.println("Button pressed for " + String(elapsed) + " ms");
  }
  return elapsed;
}

void handleTTL() {
  ulong now = millis();
  long elapsed = (now - lastTTLCheck);
  if (elapsed < TTL_CHECK_PERIOD_MS) {
    return;
  } else {
    lastTTLCheck = now;
  }
  for (auto& item: statusMap) {
    PanelStatus& pstatus = item.second;
    if (isPanelOn(pstatus.state)) {
      long elapsedInSconds = (elapsed/1000);
      long remaining = std::max((long)0, pstatus.ttl - elapsedInSconds); // in seconds
      if (remaining <= 0) {
        pstatus.state = PANEL_OFF;
        pstatus.ttl = 0;
        Serial.println("Tuning panel '" + item.first + "' off due to TTL");
        applySignStatus(pstatus);
      } else {
        pstatus.ttl = (ushort)remaining;
      }
    }
  }
}

bool isLedRGB(LedTypes led) {
  return led == RGB_RED ||
         led == RGB_GREEN ||
         led == RGB_BLUE;
}

bool isPanelOn(PanelState state) {
  return state == PANEL_ON;
}

void turnPanelOn(PanelStatus& panel) {
  if (panel.state == PANEL_ON) {
    return;
  }
  panel.state = PANEL_ON;
  panel.ttl = getAutoTurnOffPeriod();
  applySignStatus(panel);
}

void turnAllPanelsOff() {
  for (auto& item: statusMap) {
    turnPanelOff(item.second);
  }
}

void turnPanelOff(PanelStatus& panel) {
  if (panel.state == PANEL_OFF) {
    return;
  }
  panel.state = PANEL_OFF;
  panel.ttl = 0;
  applySignStatus(panel);
}

void turnNextPanelOn() {
  bool turnNextOn = isAllPanelsOff(); // if all is off, turn first panel on
  bool nextIsOn = false;
  for (auto& item: statusMap) {
    PanelStatus& panel = item.second;
    if (panel.state == PANEL_DISABLED) {
      continue;
    } else if (turnNextOn && !nextIsOn) {
      turnPanelOn(panel);
      turnNextOn = false;
      nextIsOn = true;
    } else if (panel.state == PANEL_ON) {
      if (!nextIsOn) {
        turnNextOn = true;
      }
      turnPanelOff(panel);
    }
  }

  bool shouldTurnFirstOn = turnNextOn && !nextIsOn; // if no panel was on or if last panel was on, so turn the on the first
  if (shouldTurnFirstOn) {
    for (auto& item: statusMap) {
      PanelStatus& panel = item.second;
      if (panel.state != PANEL_DISABLED) {
        turnPanelOff(panel);
        break;
      }
    }
  }
}

bool isAllPanelsOff() {
  for (auto& item: statusMap) {
    PanelStatus panel = item.second;
    if (panel.state == PANEL_DISABLED) {
      continue;
    } else if (panel.state != PANEL_OFF) {
      return false;
    }
  }
  return true;
}

void applySignStatus() {
  for (auto& item: statusMap) {
    applySignStatus(item.second);
  }
}

void applySignStatus(PanelStatus panel) {
  String name = panel.name;
  PanelState state = panel.state;
  BlinkingType blinking = panel.blinking;
  String color = panel.color;
  byte intensity = panel.intensity;

  Serial.println("Set panel '" + name + "' state to '" + panelStateToString(panel.state) + "'");
  LedTypes firstLed = panelSetupMap.at(name).begin()->first;
  bool panelIsRGB = isLedRGB(firstLed);
  if (panelIsRGB) {
    int r, g, b;
    if (!colorToRGB(color, intensity, r, g, b)) {
      Serial.println(" could not update to color '" + color + "': unknown color");
      return;
    }
    Serial.println(" set color RGB (" + String(r) + "," + String(g) + "," + String(b) + ")");
    byte r_gpio = panelSetupMap.at(name).at(RGB_RED).gpio;
    setLightState(r_gpio, state, blinking, r);
    byte g_gpio = panelSetupMap.at(name).at(RGB_GREEN).gpio;
    setLightState(g_gpio, state, blinking, g);
    byte b_gpio = panelSetupMap.at(name).at(RGB_BLUE).gpio;
    setLightState(b_gpio, state, blinking, b);

    updateAlexaDevice(name, isPanelOn(state), intensity, (byte)r, (byte)g, (byte)b);
    return;
  }

  if (color.startsWith("#")) {
    Serial.println(" panel does not support RGB color");
    return;
  }
  if (!ColorToLed.count(color)) {
    Serial.println(" could not find gpio for panel '" + name + "'");
    return;
  }

  LedTypes ledToSet = ColorToLed.at(color);
  for (auto& item: panelSetupMap.at(name)) {
    LedTypes led = item.first;
    byte gpio = item.second.gpio;
    if (led == ledToSet) { 
      setLightState(gpio, state, blinking, intensity);
    } else {
      setLightState(gpio, PANEL_OFF); // turn off any other color
    }
  }
  updateAlexaDevice(name, isPanelOn(state), intensity);
}

String getSignStatus() {
  DynamicJsonDocument doc(2000);
  doc["timestamp"] = getEpochTime();
  doc["uptime"] = getUptime();
  doc["inactiveTime"] = getInactiveTime();
  doc["signModel"] = SIGN_MODEL;
  doc["firmwareVersion"] = String(FW_MAJOR) + "." + String(FW_MINOR);

  JsonArray statusArray = doc.createNestedArray("panels");
  for (auto& item: statusMap) {
    PanelStatus pstatus = item.second;
    DynamicJsonDocument status(300);
    status["name"] = pstatus.name;
    status["state"] = panelStateToString(pstatus.state);
    status["blinking"] = blinkingTypeToString(pstatus.blinking);
    status["color"] = pstatus.color;
    status["intensity"] = pstatus.intensity;
    status["ttl"] = pstatus.ttl;
    statusArray.add(status);
  }

  String message = "";
  serializeJsonPretty(doc, message);
  return message;
}

ParsingResult setSignStatus(String statusJson) {
  if (statusJson.isEmpty()) {
    return JSON_PARSING_ERROR;
  }
  DynamicJsonDocument doc(2000);
  DeserializationError err = deserializeJson(doc, statusJson);
  if (err) {
    Serial.print("Deserialization error: ");
    Serial.println(err.f_str());
    return JSON_PARSING_ERROR;  
  }
  JsonObject root = doc.as<JsonObject>();
  if (!root.containsKey("panels") || !root["panels"].is<JsonArray>()) {
    Serial.println("missing 'panels' field");
    return JSON_MISSING_PANELS_FIELD;
  }
  JsonArray panels = root["panels"].as<JsonArray>();
  Serial.println("Panels to update: " + String(panels.size()));
  for (JsonObject panel : panels) {
    if (!panel.containsKey("name") || panel["name"].as<String>().isEmpty()) {
      Serial.println("missing 'name' field");
      return JSON_MISSING_NAME_FIELD;
    }
    if (!panel.containsKey("state")) {
      Serial.println("missing 'state' field");
      return JSON_MISSING_STATE_FIELD;
    }
    String name = panel["name"];
    String state = panel["state"];
    String color = panel["color"] | "";
    String intensity = panel["intensity"] | "";
    String blinking = panel["blinking"] | "";
    ushort ttl = (ushort)String(panel["ttl"] | "0").toInt();

    setPanelStatus(name, state, color, intensity, blinking, ttl);
  }
  return PARSE_SUCCESS;
}

void setPanelStatus(String name, String state, String color, String intensity, String blinking, ushort ttl) {
  for (auto& item: statusMap) {
    PanelStatus& panel = item.second;
    if (!name.equals(panel.name)) {
      continue;
    }
    if (!state.isEmpty()) {
      panel.state = parsePanelState(state);
    }
    if (!blinking.isEmpty()) {
      panel.blinking = parseBlinkingType(blinking);
    }
    if (!color.isEmpty()) {
      panel.color = color;
    }
    if (!intensity.isEmpty()) {
      panel.intensity = getIntensityByte(intensity);
    }
    if (isPanelOn(panel.state)) {
      panel.ttl = ttl > 0 ? ttl : getAutoTurnOffPeriod();
    } else {
      panel.ttl = 0;
    }

    applySignStatus(panel);
    return;
  }
  Serial.println("Panel does not exists! '" + name + "' | " + color);
}

byte getIntensityByte(String value) {
  int intValue = value.toInt();
  if (intValue > 0 && intValue < 255) {
    return (byte)intValue;
  } else {
    return 255;
  }
}

bool colorToRGB(String color, byte intensity, int& r, int& g, int& b) {
  if (color.equals("white")) {
    r = intensity;
    g = intensity;
    b = intensity;
  } else if (color.equals("red")) {
    r = intensity;
    g = 0;
    b = 0;
  } else if (color.equals("green")) {
    r = 0;
    g = intensity;
    b = 0;
  } else if (color.equals("blue")) {
    r = 0;
    g = 0;
    b = intensity;
  } else if (color.equals("yellow")) {
    r = intensity;
    g = intensity;
    b = 0;
  } else if (color.startsWith("#") && color.length() == 7) {
    long hexValue = (long) strtol( &color[1], NULL, 16);
    float alpha = (float)intensity;
    r = ((hexValue >> 16) & 0xFF) * alpha / 255.0; // Extract the RR byte
    g = ((hexValue >> 8) & 0xFF) * alpha / 255.0; // Extract the GG byte
    b = ((hexValue) & 0xFF) * alpha / 255.0; // Extract the BB byte
  } else {
    return false;
  }
  return true;
}

String panelStateToString(PanelState state) {
  switch (state) {
    default:
    case PANEL_DISABLED:
      return "disabled";
    case PANEL_OFF:
      return "off";
    case PANEL_ON:
      return "on";
  }
}

PanelState parsePanelState(String state) {
  if (state.equals("off")) {
    return PANEL_OFF;
  } else if (state.equals("on")) {
    return PANEL_ON;
  } else {
    return PANEL_DISABLED;
  }
}

String blinkingTypeToString(BlinkingType blinking) {
  switch (blinking) {
    default:
    case BLINKING_OFF:
      return "off";
    case BLINKING_NORMAL:
      return "on";
    case BLINKING_FAST:
      return "on-fast";
    case BLINKING_SLOW:
      return "on-slow";
  }
}

BlinkingType parseBlinkingType(String blinking) {
  if (blinking.equals("on")) {
    return BLINKING_NORMAL;
  } else if (blinking.equals("on-fast")) {
    return BLINKING_FAST;
  } else if (blinking.equals("on-slow")) {
    return BLINKING_SLOW;
  } else /*if (blinking.equals("off"))*/ {
    return BLINKING_OFF;
  }
}
