/*
  A sign is defined by a list of panels, which are defined by a light color (red, white, blue, etc), 
  state (on, off), pattern (blinking), and intensty (0 - 100%).

  Each sign is defined by the following structure:

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
        "colors": ["white"],
        "intensities": [10, 50, 100],
        "states": ["off", "on-solid"],
      },
      {
        "name": "do-not-disturb",
        "colors": ["red"],
        "intensities": [100],
        "states": ["off", "on-solid", "on-blinking", "on-blinking-fast"],
      },
      {
        "name": "camera",
        "colors": ["yellow"],
        "intensities": [100],
        "states": ["off", "on-solid", "on-blinking", "on-blinking-fast"],
      },
      {
        "name": "microphone",
        "colors": ["yellow"],
        "intensities": [100],
        "states": ["off", "on-solid", "on-blinking", "on-blinking-fast"],
      }
    ]
  }

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
        "intensity": 100,
        "state": "on-solid",
      },
      {
        "name": "camera",
        "color": "yellow",
        "intensity": 100,
        "state": "on-blinking",
      },
      {
        "name": "microphone",
        "color": "yellow",
        "intensity": 100,
        "state": "off",
      }
    ]
  } 

*/

#define ON HIGH
#define OFF LOW

void setupSign() {
  for (int i=0; i < panelSetupsLen; i++) {
    ushort pin = panelSetups[i].gpio_pin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, OFF);
  }
  updateStatus();
}

bool isOn(String state) {
  return String("on-solid").equals(state);
}

bool isOff(String state) {
  return String("off").equals(state);
}

void updateStatus() {
  for (int p; p < panelStatusLen; p++) {
    String name = panelStatus[p].name;
    String state = panelStatus[p].state;
    String color = panelStatus[p].color;
    ushort pin = findPanelSetupPin(name, color);
    if (pin == 9999) {
      Serial.print("Could not update panel ");
      Serial.println(name);
    }
    if (isOn(state)) {
      digitalWrite(pin, ON);
      Serial.print("Updated panel ");
      Serial.print(name);
      Serial.println(" to ON state");
    } else if (isOff(state)) {
      digitalWrite(pin, OFF);
      Serial.print("Updated panel ");
      Serial.print(name);
      Serial.println(" to OFF state");
    } else {
      digitalWrite(pin, OFF);
      Serial.print("Updated panel ");
      Serial.print(name);
      Serial.println(" to default OFF state");
    }
  }
}

ushort findPanelSetupPin(String name, String color) {
  int setupCount = sizeof(panelSetups)/sizeof(PanelSetup);
  for (int i=0; i < setupCount; i++) {
    if (!name.equals(panelSetups[i].name)) {
      continue;
    }
    if (!color.equals(panelSetups[i].color)) {
      continue;
    }
    return panelSetups[i].gpio_pin;
  }
  return 9999;
}

String getSignStatus() {
  DynamicJsonDocument doc(2000);
  doc["timestamp"] = millis();
  doc["model"] = SIGN_MODEL;
  doc["serial-number"] = SIGN_SN;
  doc["hw-model"] = ARDUINO_BOARD_ID;
  doc["hw-version"] = ARDUINO_ESP8266_RELEASE;
  doc["server-version"] = String(FW_MAJOR) + "." + String(FW_MINOR);
  doc["manufacturing-date"] = "2023-06-01";

  JsonArray statusArray = doc.createNestedArray("panels");
  for (int p=0; p < panelStatusLen; p++) {
    DynamicJsonDocument status(500);
    status["name"] = panelStatus[p].name;
    status["state"] = panelStatus[p].state;
    status["color"] = panelStatus[p].color;
    status["intensity"] = panelStatus[p].intensity;
    statusArray.add(status);
  }

  String message = "";
  serializeJsonPretty(doc, message);
  return message;
}

void setStatus(String statusJson) {

}

