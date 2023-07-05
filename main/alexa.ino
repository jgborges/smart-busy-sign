#include <fauxmoESP.h>

fauxmoESP fauxmo;

void setupAlexa() {
  Serial.print("Initializing Alexa support...");
  //fauxmo.createServer(false); // required when integrating with existing webserver
  fauxmo.addDevice("Busy panel");
  fauxmo.addDevice("Camera panel");
  fauxmo.addDevice("Mic panel");
  fauxmo.addDevice("Do not disturb");

  //fauxmo.setPort(80); // required for gen3 devices
  fauxmo.enable(true);

  Serial.println("Done");

  fauxmo.onSetState([](unsigned char device_id, const char* device_name, bool state, unsigned char value) {
      Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
  });
}

void handleAlexa() {
  fauxmo.handle();
}