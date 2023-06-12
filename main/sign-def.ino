/*
   SIGN DEFINITIONS
*/

#define SIGN_MODEL "smart-busy-sign_beta"
#define SIGN_SN 10001

const short RESET = D6;

const PanelSetup SMART_BUSY_SIGN_BETA_PINS[] = {
  { "busy", "white", LED_BUILTIN },
  //{ "busy", "white", D1 },
  { "busy", "red", D2 },
  { "do-not-disturb", "red", D3 },
  { "camera", "yellow", D5 },
  { "microphone", "yellow", D6 },
};

// initialize with default status
PanelStatus SMART_BUSY_SIGN_BETA_DEFAULT_STATUS[] = {
  { "busy", "off", "white", 255 },
  { "do-not-disturb", "off", "red", 255 },
  { "camera", "off", "yellow", 255 },
  { "microphone", "off", "yellow", 255 },
};

const PanelSetup* panelSetups = SMART_BUSY_SIGN_BETA_PINS;
const int panelSetupsLen = 5; //sizeof(SMART_BUSY_SIGN_BETA_PINS)/sizeof(PanelSetup);

PanelStatus* panelStatus = SMART_BUSY_SIGN_BETA_DEFAULT_STATUS;
const int panelStatusLen = 4; //sizeof(SMART_BUSY_SIGN_BETA_DEFAULT_STATUS)/sizeof(PanelStatus);