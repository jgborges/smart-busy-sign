#include <fauxmoESP.h>

fauxmoESP fauxmo;

void setupAlexa() {
  Serial.println("Initializing Alexa support...");
  fauxmo.createServer(false);
  fauxmo.setPort(80); // required for gen3 devices

  fauxmo.addDevice("busy");
  fauxmo.setDeviceUniqueId(fauxmo.getDeviceId("busy"), "panel_busy_0");
  //fauxmo.addDevice("camera");
  //fauxmo.addDevice("mic");
  //fauxmo.addDevice("do-not-disturb");
  fauxmo.enable(true);

  Serial.println("Done");

  fauxmo.onSetState([](unsigned char device_id, const char* device_name, bool state, unsigned char value) {
      Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
  });
}

void handleAlexa() {
  fauxmo.handle();
}