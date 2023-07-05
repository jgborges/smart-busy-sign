#include "time.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "user_interface.h"

#define MAX_DEEP_SLEEP_IN_MINUTES 60

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

ulong lastSleepCheck;
ulong uptimeOffset;

void setupTime() {
  // required for light sleep
  //wifi_fpm_set_wakeup_cb(fpm_wakup_cb_func1);
  // enable light sleep
  //wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
  //wifi_fpm_open();

  const int offset = getTzOffsetInSecods();
  const int updateInterval = 3600000; // 1h
  timeClient.begin();
  timeClient.setTimeOffset(offset);
  timeClient.setUpdateInterval(updateInterval);
  timeClient.update();

  lastSleepCheck = millis();
  upTimeOffset = millis();

  if (isWakingFromDeepSleep()) {
    Serial.println("Waking up from deep sleep");
    handleAutoSleep();
  }
}

ulong getEpochTime() {
  if (timeClient.isTimeSet()) {
    Serial.println("Time: " + timeClient.getFormattedTime());
    return timeClient.getEpochTime();
  } else {
    return millis();
  }
}

ulong getUptime() {
  return millis() - uptimeOffset;
}

void handleSleep() {
  timeClient.update();

  ulong now = millis();
  long elapsed = (now - lastSleepCheck);
  if (elapsed > SLEEP_CHECK_PERIOD_MS) {
    lastSleepCheck = now;
    bool shouldPrintRemainingTime = ((elapsed/1000) % 10) == 0;
    handleAutoSleep();
  }
}

void handleAutoSleep() {
  return handleAutoSleep(false);
}

void handleAutoSleep(bool printRemainingTime) {
  byte hStart, hEnd;
  if (!getAutoSleepHours(hStart, hEnd) || !timeClient.isTimeSet()) {
    return;
  }

  int hCurrent = timeClient.getHours();
  bool shouldSleep = (hStart < hEnd) ? 
    (hCurrent >= hStart && hCurrent < hEnd) :
    (hCurrent >= hStart || hCurrent < hEnd);

  if (shouldSleep) {
    ulong sleepTimeInHours = hStart < hEnd ? (hEnd - hStart) : (24 - hStart + hEnd);
    ulong sleepTimeInMicroSeconds = std::min(sleepTimeInHours*60, (ulong)MAX_DEEP_SLEEP_IN_MINUTES)*60*1000*1000; // in microseconds
    Serial.println("Entering deep sleep for " + String(sleepTimeInHours) + " hours");
    turnAllLightsOff();
    ESP.deepSleep(sleepTimeInMicroSeconds);
  } else if (printRemainingTime) {
    int mCurrent = timeClient.getMinutes();
    ulong waitTimeInMinutes = hCurrent < hStart ? ((hStart-hCurrent)*60) - mCurrent : ((24-hCurrent)*60) - mCurrent + (hStart*60);
    Serial.println("Auto sleep enabled. Will sleep in " + String(waitTimeInMinutes) + " minutes");
  }
}

void deepSleep(ulong timeToSleep) {
  digitalWrite(D0, LOW);
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