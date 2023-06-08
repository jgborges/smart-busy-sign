#ifndef AP_SSID
#define AP_SSID "Smart Busy Sign"
#define AP_PSK "smartsign"
#endif

#define FIFTEEN_SECONDS 10000

const char* hostname = "smart-busy-sign";

void setupWifi() {
  // prepare LED
  bool led = true;
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, ON);

  bool isConnected = false;
  if (storage.isWifiSet() && storage.wifiMode == WIFI_STA) {
    Serial.println();
    Serial.println(F("Stored WiFi settings - Client mode"));
    isConnected = setupSTA(storage.wifiSsid, storage.wifiPwd);
  }

  if (storage.isWifiSet() && storage.wifiMode == WIFI_STA) {
    Serial.println();
    Serial.println(F("Stored WiFi settings - AP mode"));
    setupAP(storage.wifiSsid, storage.wifiPwd);
    isConnected = true;
  }

  if (!isConnected) {
    Serial.println();
    Serial.println(F("Default WiFi settings - AP mode"));
    setupAP(AP_SSID, AP_PSK);
    // Print the AP IP address
    Serial.println(WiFi.softAPIP());
  } else {
    // Print the IP address
    Serial.println(WiFi.localIP());
  }

  // Set up mDNS responder:
  // - argument is the domain name set to hostname, resulting in 
  //   the fully-qualified domain name "<hostname>.local"
  if (!MDNS.begin(hostname)) {
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println(F("mDNS responder started"));

  // Set up NetBIOS responder:
  // - argument is the hostname
  if (!NBNS.begin(hostname)) {
    Serial.println("Error setting up NetBIOS!");
  }
  Serial.println(F("NetBIOS started"));
}

bool setupSTA(const char* ssid, const char* password) {
  // Connect to WiFi network
  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  bool led = true;
  ulong start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    ulong elapsed = millis() - start;
    if (elapsed > FIFTEEN_SECONDS) {
      Serial.println();
      Serial.println(F("Could not connect to Wifi"));
      return false;
    } else {
      // blink LED to indicate activity
      Serial.print(F("."));
      led = !led;
      digitalWrite(LED_BUILTIN, led ? ON : OFF);
    }
  }
  Serial.println();
  Serial.println(F("WiFi connected"));

  digitalWrite(LED_BUILTIN, OFF); // keep LED off to indicate successful connection
  return true;
}

void setupAP(const char* ssid, const char* password) {
  // Create the WiFi network
  Serial.print(F("Setting AP network "));
  Serial.println(ssid);
  
  WiFi.mode(WIFI_AP);
  WiFi.hostname(hostname);
  WiFi.softAP(ssid, password);
  
  Serial.println();
  Serial.println(F("WiFi connected"));

  digitalWrite(LED_BUILTIN_AUX, OFF); // keep LED off to indicate successful connection
}
