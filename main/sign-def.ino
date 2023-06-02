/*
   Basic Types
*/

struct PanelSetup {
  String name;
  String color;
  ushort gpio_pin;
};

struct PanelStatus {
  String name;
  String state;
  String color;
  ushort intensity;
};

/*
   SIGN DEFINITIONS
*/

#define SIGN_MODEL "smart-busy-sign_beta"
#define SIGN_SN 10001

const short RESET = D6;

const PanelSetup SMART_BUSY_SIGN_BETA_PINS[] = {
  { "busy", "white", D1 },
  { "busy", "red", D2 },
  { "do-not-disturb", "red", D3 },
  { "camera", "white", D4 },
  { "microphone", "white", D5 },
};

// initialize with default status
const PanelStatus SMART_BUSY_SIGN_BETA_DEFAULT_STATUS[] = {
  { "busy", "off", "white", 100 },
  { "do-not-disturb", "off", "red", 100 },
  { "camera", "off", "yellow", 100 },
  { "microphone", "off", "yellow", 100 },
};

const PanelSetup* panelSetups = SMART_BUSY_SIGN_BETA_PINS;
const PanelStatus* panelStatus = SMART_BUSY_SIGN_BETA_DEFAULT_STATUS;
