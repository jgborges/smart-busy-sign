
struct BlinkState {
  bool isBlinking;
  bool isOn;
  int interval;
  int lastStateChangeMillis;
};

BlinkState blinkStates[20]; // ESP8266 has 16 GPIOs

void blinkingOn(ushort gpioPin) {

}

void blinkingOff(ushort gpioPin) {

}

void handleBlinking() {

}