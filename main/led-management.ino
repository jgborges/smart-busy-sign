
struct GpioState {
  PanelState state;
  BlinkingType blinking;
  byte intensity;
  bool isPosAnode;
};

// blinking intervals, coded as { <iterval on>, <interval off> } in milliseconds
const std::map<BlinkingType, std::array<ushort, 2>> BlinkIntervals = {
  { BLINKING_SLOW,   { 900, 600 }}, // 1.5 second cycle
  { BLINKING_NORMAL, { 600, 300 }}, // 0.9 second cyble
  { BLINKING_FAST,   { 150, 150 }}  // 0.3 second cycle
};

struct BlinkingState {
  // blinking params
  ushort intervalOn;
  ushort intervalOff;
  // manage on/off state for blinking effect
  bool isLightOn;
  ulong lastStateChangeMillis;
};

GpioState gpioStates[MAX_PINS]; // ESP8266 has 16 GPIOs
BlinkingState blinkingStates[NumBlinkingTypes];

ulong lastBlinkCheck;

void setupGpio(const PanelSetupMap& setup);

void setupGpio(const PanelSetupMap& setup) {
  // initialize default values
  for (int gpio=0; gpio < MAX_PINS; gpio++) {
    gpioStates[gpio].state = PANEL_DISABLED;
    gpioStates[gpio].intensity = 255;
    gpioStates[gpio].blinking = BLINKING_OFF;
  }

  for (auto& item: setup) {
    for (auto& led: item.second) {
      byte gpio = led.second.gpio;
      pinMode(gpio, OUTPUT);
      gpioStates[gpio].state = PANEL_OFF;
      gpioStates[gpio].isPosAnode = led.second.isPosAnode;
    }
  }

  enablePanels();
  
  // initialize and configure blinking states
  int now = millis();
  for (int i=0; i < NumBlinkingTypes; i++) {
    std::array<ushort, 2> interval = BlinkIntervals.at((BlinkingType)i);
    blinkingStates[i].intervalOn = interval[0];
    blinkingStates[i].intervalOff = interval[1];
    blinkingStates[i].isLightOn = false;
    blinkingStates[i].lastStateChangeMillis = now;
  }

  lastBlinkCheck = now;
}

void turnLightOn(uint8_t gpio, byte intensity = 255) {
  bool isPWM = intensity > 0 && intensity < 255;
  bool isPosAnode = gpioStates[gpio].isPosAnode;
  if (intensity == 0) {
    turnLightOff(gpio);
  } else if (isPWM) {
    byte value = isPosAnode ? intensity : (255-intensity);
    analogWrite(gpio,  value);
  } else {
    digitalWrite(gpio, isPosAnode ? HIGH : LOW);
  }
}

void turnLightOff(uint8_t gpio) {
  bool isPosAnode = gpioStates[gpio].isPosAnode;
  digitalWrite(gpio, isPosAnode ? LOW : HIGH);
}

void setLightState(uint8_t gpio, PanelState newState, BlinkingType newBlinking = BLINKING_OFF, byte newIntensity = 255) {
  Serial.print(" gpio " + String(gpio) + ": ");
  if (gpio >= MAX_PINS) {
    Serial.println("invalid gpio pin");
    return;
  }

  // This is common for RGB cases
  if (newIntensity == 0) {
    newState = PANEL_OFF;
  }

  GpioState& light = gpioStates[gpio];
  if (light.state == newState && light.intensity == newIntensity && light.blinking == newBlinking) {
    Serial.println("no change in state");
    return; // nothing is changing
  }

  light.state = newState;
  light.intensity = newIntensity; 
  light.blinking = newBlinking;

  if (!isPanelOn(newState) || newIntensity == 0) {
    Serial.println("turn off");
    turnLightOff(gpio);
  } else {
    // set light intensity
    Serial.println("turn on intensity " + String(newIntensity));
    turnLightOn(gpio, light.intensity);
  }
}

void enablePanels() {
  // turn all lights on, when initializing board, then off
  Serial.print("Enabling all panels...");
  if (!isWakingFromDeepSleep()) {
    turnAllLightsOn();
    delay(3000); 
  }
  turnAllLightsOff();
  Serial.println("done!"); 
}

void turnAllLightsOn() {
  for (int gpio=0; gpio < MAX_PINS; gpio++) {
    if (gpioStates[gpio].state == PANEL_DISABLED) {
      continue;
    }
    turnLightOn(gpio, 255);
  }
}

void turnAllLightsOff() {
  for (int gpio=0; gpio < MAX_PINS; gpio++) {
    if (gpioStates[gpio].state == PANEL_DISABLED) {
      continue;
    }
    turnLightOff(gpio);
  }
}

bool isAllLightsOff() {
  for (int gpio=0; gpio < MAX_PINS; gpio++) {
    if (gpioStates[gpio].state == PANEL_DISABLED) {
      continue;
    } else if (gpioStates[gpio].state != PANEL_OFF) {
      return false;
    }
  }
  return true;
}

void handleBlinking() {
  ulong now = millis();
  long elapsed = (now - lastBlinkCheck);
  if (elapsed < BLINK_CHECK_PERIOD_MS) {
    return;
  } else {
    lastBlinkCheck = now;
  }
  
  for (int i=0; i < NumBlinkingTypes; i++) {
    BlinkingType blinkType = (BlinkingType)i;
    BlinkingState& blink = blinkingStates[i];
    long blinkElapsed = now - blink.lastStateChangeMillis;
    ulong interval = blink.isLightOn ? blink.intervalOn : blink.intervalOff;
    bool trigger = blinkElapsed > interval;
    if (trigger) {
      blink.isLightOn = !blink.isLightOn;
      for (int gpio=0; gpio < MAX_PINS; gpio++) {
        GpioState& light = gpioStates[gpio];
        if (!isPanelOn(light.state) || light.blinking != blinkType) {
          continue;
        }
        if (blink.isLightOn) {
          turnLightOn(gpio, light.intensity);
        } else {
          turnLightOff(gpio);
        }
      }
      blink.lastStateChangeMillis = now;
    }
  }
}
