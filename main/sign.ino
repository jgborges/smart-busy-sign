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

#define ON LOW
#define OFF HIGH

#define NO_GPIO 255

void setupSign() {
  setupGpio();

  Serial.print("Enabling all panels...");
  for (int i=0; i < panelSetupsLen; i++) {
    byte gpio = panelSetups[i].gpio;
    pinMode(gpio, OUTPUT);
    digitalWrite(gpio, ON);
  }
  delay(3000); // turn all lights on after turn on
  Serial.println("done!"); 

  // appy default sign status
  applySignStatus();
}

bool isOn(String state) {
  return String("on-solid").equals(state);
}

bool isOff(String state) {
  return String("off").equals(state);
}

void applySignStatus() {
  for (int i=0; i < panelStatusLen; i++) {
    String name = panelStatus[i].name;
    String state = panelStatus[i].state;
    String color = panelStatus[i].color;
    byte intensity = panelStatus[i].intensity;

    uint8_t gpio = findPanelSetupPin(name, color);
    if (gpio == NO_GPIO) {
      Serial.println("Could not find panel " + name);
    } else {
      setLightState(gpio, state, intensity);
      Serial.println("Updated panel " + name + " to '" + state + "' state");
    }
  }
}

byte findPanelSetupPin(String name, String color) {
  for (int i=0; i < panelSetupsLen; i++) {
    if (!name.equals(panelSetups[i].name)) {
      continue;
    }
    if (!color.isEmpty() && !color.equals(panelSetups[i].color)) {
      continue;
    }
    return panelSetups[i].gpio;
  }
  return NO_GPIO;
}

String getSignStatus() {
  DynamicJsonDocument doc(2000);
  doc["timestamp"] = getEpochTime();
  doc["model"] = SIGN_MODEL;
  doc["serial-number"] = SIGN_SN;
  doc["hw-model"] = ARDUINO_BOARD;
  doc["hw-version"] = ARDUINO_ESP8266_RELEASE;
  doc["server-version"] = String(FW_MAJOR) + "." + String(FW_MINOR);
  doc["manufacturing-date"] = "2023-06-01";

  JsonArray statusArray = doc.createNestedArray("panels");
  for (int p=0; p < panelStatusLen; p++) {
    DynamicJsonDocument status(300);
    status["name"] = panelStatus[p].name;
    status["state"] = panelStatus[p].state;
    status["color"] = panelStatus[p].color;
    status["intensity"] = panelStatus[p].intensity;
    status["ttl"] = panelStatus[p].ttl;
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
    Serial.println("missing panels field");
    return JSON_MISSING_PANELS_FIELD;
  }
  JsonArray panels = root["panels"].as<JsonArray>();
  Serial.println("Panels to update: " + String(panels.size()));
  for (JsonObject panel : panels) {
    Serial.println("Iterating panels to update!");
    Serial.println(String(panel["name"]));
    if (!panel.containsKey("name") || panel["name"].as<String>().isEmpty()) {
      Serial.println("missing name field");
      return JSON_MISSING_NAME_FIELD;
    }
    if (!panel.containsKey("state")) {
      Serial.println("missing color or state field");
      return JSON_MISSING_STATE_FIELD;
    }
    String name = panel["name"];
    String state = panel["state"];
    String color = panel["color"] | "";
    String intensity = panel["intensity"] | "";
    setPanelStatus(name, color, state, intensity);
  }
  applySignStatus();
  return PARSE_SUCCESS;
}

void setPanelStatus(String name, String color, String state, String intensity) {
  for (int i=0; i < panelStatusLen; i++) {
    if (!name.equals(panelStatus[i].name)) {
      continue;
    }
    if (!color.isEmpty()) {
      panelStatus[i].color = color;
    }
    if (!intensity.isEmpty()) {
      panelStatus[i].intensity = getIntensityByte(intensity);
    }
    panelStatus[i].state = state;
    Serial.println("Panel " + name + " new state: " + state);
    return;
  }
  Serial.println("Did not find panel to set");
}

byte getIntensityByte(String value) {
  int intValue = value.toInt();
  if (intValue > 0 && intValue < 255) {
    return (byte)intValue;
  } else {
    return 255;
  }
}
