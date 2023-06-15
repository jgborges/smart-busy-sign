#define ON LOW
#define OFF HIGH

// 0.3 second cycle
#define BLINK_FAST_INTERVAL_ON 150
#define BLINK_FAST_INTERVAL_OFF 150

// 0.9 second cycle
#define BLINK_DEFAULT_INTERVAL_ON 600
#define BLINK_DEFAULT_INTERVAL_OFF 300

// 1.5 second cycle
#define BLINK_SLOW_INTERVAL_ON 900
#define BLINK_SLOW_INTERVAL_OFF 600

struct GpioState {
  PanelState state;
  byte intensity;

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

ulong lastBlinkCheck;
const int NUM_BLINKING_TYPES = BLINKING_OFF - BLINKING_NORMAL; // we don't add OFF as an option

struct BlinkingState {
  // blinking params
  ushort intervalOn;
  ushort intervalOff;
  // manage on/off state for blinking effect
  bool isLightOn;
  ulong lastStateChangeMillis;
};

GpioState gpioStates[NUM_DIGITAL_PINS]; // ESP8266 has 16 GPIOs
BlinkingState blinkingStates[NUM_BLINKING_TYPES];

void setupGpio() {
  for (int gpio=0; gpio < NUM_DIGITAL_PINS; gpio++) {
    gpioStates[gpio].state = PANEL_DISABLED;
    gpioStates[gpio].intensity = 255;
  }
  
  int now = millis();
  blinkingStates[BLINKING_FAST].intervalOn = BLINK_FAST_INTERVAL_ON;
  blinkingStates[BLINKING_FAST].intervalOff = BLINK_FAST_INTERVAL_OFF;
  blinkingStates[BLINKING_FAST].isLightOn = false;
  blinkingStates[BLINKING_FAST].lastStateChangeMillis = now;

  blinkingStates[BLINKING_NORMAL].intervalOn = BLINK_DEFAULT_INTERVAL_ON;
  blinkingStates[BLINKING_NORMAL].intervalOff = BLINK_DEFAULT_INTERVAL_OFF;
  blinkingStates[BLINKING_NORMAL].isLightOn = false;
  blinkingStates[BLINKING_NORMAL].lastStateChangeMillis = now;

  blinkingStates[BLINKING_SLOW].intervalOn = BLINK_SLOW_INTERVAL_ON;
  blinkingStates[BLINKING_SLOW].intervalOff = BLINK_SLOW_INTERVAL_OFF;
  blinkingStates[BLINKING_SLOW].isLightOn = false;
  blinkingStates[BLINKING_SLOW].lastStateChangeMillis = now;

  lastBlinkCheck = now;
}

bool isOn(PanelState state) {
  return state == PANEL_ON;
}

bool isOff(PanelState state) {
  return state == PANEL_OFF || state == PANEL_DISABLED;
}

void turnLightOn(uint8_t gpio, byte intensity = 255) {
  bool isPWM = intensity > 0 && intensity < 255;
  if (isPWM) {
    analogWrite(gpio, 255-intensity); // we turn LED on by setting LOW to cathode, so the we need the complement of 255 for intensity
  } else {
    digitalWrite(gpio, ON);
  }
}

void turnLightOff(uint8_t gpio) {
  digitalWrite(gpio, OFF);
}

void setLightState(uint8_t gpio, PanelState newState, byte newIntensity = 255) {
  Serial.print(" gpio " + String(gpio) + ": ");
  if (gpio >= NUM_DIGITAL_PINS) {
    Serial.println("invalid gpio pin");
    return;
  }

  GpioState& light = gpioStates[gpio];
  if (light.state == newState && light.intensity == newIntensity) {
    Serial.println("no change in state");
    return; // nothing is changing
  }

  light.state = newState;
  light.intensity = newIntensity; 

  if (isOff(newState)) {
    Serial.println("turn off");
    turnLightOff(gpio);
  } else if (isOn(newState)) {
    // set light intensity
    Serial.println("turn on intensity " + String(newIntensity));
    turnLightOn(gpio, light.intensity);
  } else {
    // set blinking state
    Serial.println("turn on blinking");
    handleBlinking();
  }
}

void handleBlinking() {
  ulong now = millis();
  if ((now - lastBlinkCheck) < 100) { // we only check blink every 100 ms
    return;
  } else {
    lastBlinkCheck = now;
  }
  
  for (int i=0; i < NUM_BLINKING_TYPES; i++) {
    BlinkingType blinkType = (BlinkingType)i;
    BlinkingState& blink = blinkingStates[blinkType];
    ulong elapsed = now - blink.lastStateChangeMillis;
    ulong interval = blink.isLightOn ? blink.intervalOn : blink.intervalOff;
    bool trigger = elapsed > interval;
    if (trigger) {
      blink.isLightOn = !blink.isLightOn;
      for (int gpio=0; gpio < NUM_DIGITAL_PINS; gpio++) {
        if (gpioStates[gpio].blinkingType() != blinkType) {
          continue;
        }
        if (blink.isLightOn) {
          turnLightOn(gpio, gpioStates[gpio].intensity);
        } else {
          turnLightOff(gpio);
        }
      }
      blink.lastStateChangeMillis = now;
    }
  }
}
