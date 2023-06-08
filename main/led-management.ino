
#define ON LOW
#define OFF HIGH

// 0.3 second cycle
#define BLINK_FAST_INTERVAL_ON 150
#define BLINK_FAST_INTERVAL_OFF 150

// 1.0 second cycle
#define BLINK_DEFAULT_INTERVAL_ON 600
#define BLINK_DEFAULT_INTERVAL_OFF 400

// 1.5 second cycle
#define BLINK_SLOW_INTERVAL_ON 900
#define BLINK_SLOW_INTERVAL_OFF 600

struct GpioState {
  String state;
  byte intensity;
};

ulong lastBlinkCheck;

struct BlinkingState {
  // blinking status of gpio
  bool isBlinking;
  ushort intervalOn;
  ushort intervalOff;
  // manage on/off state for blinking effect
  bool isLightOn;
  ulong lastStateChangeMillis;
};

GpioState gpioStates[NUM_DIGITAL_PINS]; // ESP8266 has 16 GPIOs
BlinkingState gpioBlinkingStates[NUM_DIGITAL_PINS]; // ESP8266 has 16 GPIOs

void setupGpio() {
  for (int gpio=0; gpio < NUM_DIGITAL_PINS; gpio++) {
    gpioStates[gpio].state = "off";
    gpioStates[gpio].intensity = 255;
    gpioBlinkingStates[gpio].isBlinking = false;
  }
  lastBlinkCheck = millis();
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

void setLightState(uint8_t gpio, String newState, byte newIntensity = 255) {
  if (gpio >= NUM_DIGITAL_PINS) {
    Serial.println("Invalid gpio pin: " + String(gpio));
    return;
  }

  GpioState& light = gpioStates[gpio];
  BlinkingState& blink = gpioBlinkingStates[gpio];

  if (light.state.equals(newState) && light.intensity == newIntensity) {
    Serial.println("gpio " + String(gpio) + ": no change in state");
    return; // nothing is changing
  }

  light.state = newState;
  light.intensity = newIntensity;
  blink.isBlinking = false;
 
  bool isOff = newState.equals("off");
  if (isOff) {
    Serial.println("gpio " + String(gpio) + ": turn off");
    turnLightOff(gpio);
    return;
  }

  // set light intensity
  Serial.println("gpio " + String(gpio) + ": turn on intensity " + String(newIntensity));
  turnLightOn(gpio, light.intensity);
  if (newState.equals("on-solid")) {
    return;
  }

  // set blinking state
  Serial.println("gpio " + String(gpio) + ": turn on blinking");
  blink.isBlinking = true;
  blink.isLightOn = true;
  blink.intervalOn = newState.equals("on-blinking-fast") ? BLINK_FAST_INTERVAL_ON : 
                     newState.equals("on-blinking-slow") ? BLINK_SLOW_INTERVAL_ON : 
                                                           BLINK_DEFAULT_INTERVAL_ON;
  blink.intervalOff = newState.equals("on-blinking-fast") ? BLINK_FAST_INTERVAL_OFF : 
                      newState.equals("on-blinking-slow") ? BLINK_SLOW_INTERVAL_OFF : 
                                                            BLINK_DEFAULT_INTERVAL_OFF;
  blink.lastStateChangeMillis = millis();
}

void handleBlinking() {
  ulong now = millis();
  if ((now - lastBlinkCheck) < 100) { // we only check blink every 100 ms
    return;
  } else {
    lastBlinkCheck = now;
  }
  for (int gpio=0; gpio < NUM_DIGITAL_PINS; gpio++) {
    BlinkingState& blink = gpioBlinkingStates[gpio];
    if (!blink.isBlinking) {
      continue;
    }
    ulong elapsed = now - blink.lastStateChangeMillis;
    bool trigger = blink.isLightOn ? elapsed > blink.intervalOn : elapsed > blink.intervalOff;
    if (trigger) {
      blink.isLightOn = !blink.isLightOn;
      if (blink.isLightOn) {
        turnLightOn(gpio, gpioStates[gpio].intensity);
      } else {
        turnLightOff(gpio);
      }
      blink.lastStateChangeMillis = now;
    }
  }
}
