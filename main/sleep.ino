#include "time.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "user_interface.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void setupTime() {
  // required for light sleep
  wifi_fpm_set_wakeup_cb(fpm_wakup_cb_func1);
  // enable light sleep
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
  wifi_fpm_open();

  const int offset = getTzOffsetInSecods();
  const int updateInterval = 3600000; // 1h
  timeClient.begin();
  timeClient.setTimeOffset(offset);
  timeClient.setUpdateInterval(updateInterval);
}

ulong getEpochTime() {
  if (timeClient.isTimeSet()) {
    Serial.println("Time: "+ timeClient.getFormattedTime());
    return timeClient.getEpochTime();
  } else {
    return millis();
  }
}

void handleSleep() {
  timeClient.update();
}

void deepSleep() {
  long timeToSleep = 1*3600*1000; // 1h in microseconds
  ESP.deepSleep(timeToSleep);
}

void lightSleep() {
  // optional: register one or more wake-up interrupts. the chip
  // will wake from whichever (timeout or interrupt) occurs earlier
  //gpio_pin_wakeup_enable(D2, GPIO_PIN_INTR_HILEVEL);

  // sleep for 10 seconds
  long sleepTimeMilliSeconds = 200;
  // light sleep function requires microseconds
  wifi_fpm_do_sleep(sleepTimeMilliSeconds * 1000);

  // timed light sleep is only entered when the sleep command is
  // followed by a delay() that is at least 1ms longer than the sleep
  delay(sleepTimeMilliSeconds + 1);
}

// registering a on-wakeup callback
void fpm_wakup_cb_func1() {
  // ok to use blocking functions in the callback, but not
  // delay(), which appears to cause a reset
  Serial.println("Light sleep is over");
}