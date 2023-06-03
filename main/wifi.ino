const char* hostname = "smart-busy-sign";

void setupWifi() {
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);
  if (storage.isWifiSet()) {
    Serial.println(F("Stored WiFi settings"));
    const char* ssid = storage.wifi_ssid;
    const char* password = storage.wifi_pwd;
    Serial.print(F("Connecting to "));
    Serial.println(ssid);
    WiFi.begin(ssid, password);
  } else {
    // set to default debug values
    Serial.println(F("Default WiFi settings"));
    const char* ssid = STASSID;
    const char* password = STAPSK;
    Serial.print(F("Connecting to "));
    Serial.println(ssid);
    WiFi.begin(ssid, password);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "<hostname>.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin(hostname)) {
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");

  if (!NBNS.begin(hostname)) {
    Serial.println("Error setting up NetBIOS!");
  }
  Serial.println("NetBIOS started");
 
  // Print the IP address
  Serial.println(WiFi.localIP());
}