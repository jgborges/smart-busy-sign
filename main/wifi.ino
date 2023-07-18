#define FIFTEEN_SECONDS 10000
#define ON LOW
#define OFF HIGH

#define MAC_ADDR_LENGTH 6

const char* hostname = "smart-busy-sign";

void setupWifi() {
  // prepare LED
  bool led = true;
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, ON);

  String macAddr = WiFi.macAddress();
  Serial.println("Mac Address: " + macAddr);
  bool isConnected = false;

  WiFiMode_t mode;
  String ssid, psk;
  if (getWifiConfig(mode, ssid, psk)) {
    if (mode == WIFI_STA) {
      isConnected = setupSTA(ssid, psk);
    }
    if (mode == WIFI_AP) {
      setupAP(ssid, psk);
      isConnected = true;
    }
  }

  if (!isConnected) {
    Serial.println();
    Serial.println(F("Initializing with default settings..."));
    String ssid = getDefaultSSID();
    String psk = getDefaultPsk();
    setupAP(ssid, psk);
    staConnectionFailed = true; // remember so we can inform the user
    // Print the AP IP address
    Serial.println(WiFi.softAPIP());
  } else {
    // Print the IP address
    Serial.println(WiFi.localIP());
  }

  setupMDNS();
}

bool setupSTA(String ssid, String psk) {
  // Connect to WiFi network
  Serial.print("Connecting to '" + String(ssid) + " (STA)'...");
  
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);
  WiFi.begin(ssid, psk);

  bool led = true;
  ulong start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    ulong elapsed = millis() - start;
    if (elapsed < FIFTEEN_SECONDS) {
      // blink LED to indicate activity
      Serial.print(F("."));
      led = !led;
      digitalWrite(LED_BUILTIN, led ? ON : OFF);
    } else {
      Serial.println();
      Serial.println(F("Could not connect to Wifi"));
      return false;
    }
  }

  digitalWrite(LED_BUILTIN, OFF); // keep LED off to indicate successful connection
  Serial.println(F("connected!"));
  return true;
}

void setupAP(String ssid, String psk) {
  // Create the WiFi network
  Serial.print("Setting AP network '" + String(ssid) + "'...");
  
  WiFi.mode(WIFI_AP);
  WiFi.hostname(hostname);
  WiFi.softAP(ssid, psk);
#if defined(ESP8266)
  WiFi.onEvent(onWifiClientConnected, WIFI_EVENT_SOFTAPMODE_STACONNECTED);
#endif

  digitalWrite(LED_BUILTIN, OFF); // keep LED off to indicate successful connection
  Serial.println(F("connected!"));
}

void setupMDNS() {
  // Set up mDNS responder:
  // - argument is the domain name set to hostname, resulting in 
  //   the fully-qualified domain name "<hostname>.local"
  if (!MDNS.begin(hostname)) {
    Serial.println("Error setting up mDNS responder!");
  } else {
    Serial.println(F("mDNS responder started"));
  }

#if defined(ESP32)
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
#endif

  // Set up NetBIOS responder:
  // - argument is the hostname
  if (!NBNS.begin(hostname)) {
    Serial.println("Error setting up NetBIOS!");
  }
  Serial.println(F("NetBIOS started"));
}

void handleMDNS() {
  MDNS.update();
}

#if defined(ESP8266)
void onWifiClientConnected(WiFiEvent_t event) {
  Serial.println("Client connected!");
}
#endif

String getMacAddress(String separator) {
  uint8_t mac[MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = "";
  for (int i=0; i < MAC_ADDR_LENGTH; i++) {
    macID += String(mac[i], HEX);
    if (i < (MAC_ADDR_LENGTH - 1)) {
      macID += separator;
    }
  }
  macID.toUpperCase();
  return macID;
}
