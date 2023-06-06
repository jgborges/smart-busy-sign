
struct BlinkState {
  bool isBlinking;
  bool isOn;
  int interval;
  int lastStateChangeMillis;
};

BlinkState blinkStates[20]; // ESP8266 has 16 GPIOs

void blinkingOn(ushort gpio) {

}

void blinkingOff(ushort gpio) {

}

void handleBlinking() {

}