#include "time.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

#define MAX_DEEP_SLEEP_IN_MINUTES 60
#define MINUTES_TO_PRINT_SLEEP_INFO 1

#define EPOCH_DAY_OF_WEEK 4 // unix epoch started on a thursday (week starts at sunday=0)
#define SECS_PER_DAY 86400 

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

ulong lastSleepCheck;
ulong uptimeOffset;

bool isInactive;
ulong inactiveTimeStart;

void setupTime() {
  const int offset = getTzOffsetInSecods();
  const int updateInterval = 3600000; // 1h
  timeClient.begin();
  timeClient.setTimeOffset(offset);
  timeClient.setUpdateInterval(updateInterval);
  timeClient.update();

  lastSleepCheck = millis();
  uptimeOffset = millis();

  isInactive = true;
  inactiveTimeStart = millis();

  if (isWakingFromDeepSleep()) {
    Serial.println("Waking up from deep sleep");
    isInactive = !isAllLightsOff();
    inactiveTimeStart = 0; // this will cause inactive time to be "max"
    handleAutoSleep();
  }
}

/**
 * Returns timestamp in seconds
 */
ulong getEpochTime() {
  if (timeClient.isTimeSet()) {
    Serial.println("Time: " + timeClient.getFormattedTime());
    return timeClient.getEpochTime();
  } else {
    return millis()/1000;
  }
}

/**
 * Returns time in seconds device is on
 */
ulong getUptime() {
  return (millis() - uptimeOffset) / 1000;
}

/**
 * Returns time in seconds device is inactive (all panels off). 0 means not inactive.
 */
ulong getInactiveTime() {
  return isInactive ? (millis() - inactiveTimeStart) / 1000 : 0;
}

/**
 * Returns day of the week enum based on current time.
 */
DayOfWeek getDayOfWeek(ulong timestamp, int daysFromNow = 0) {
  int dow = ((timestamp / SECS_PER_DAY) + EPOCH_DAY_OF_WEEK + daysFromNow) % 7;
  dow = (dow + 7) % 7;
  DayOfWeek d = AllDaysOfWeek[dow];
  //Serial.println("day of week: " + singleDayOfWeekToString(d));
  return d;
}

void setTzOffset(int offsetInMinutes) {
  int offsetInSeconds = offsetInMinutes*60;
  timeClient.setTimeOffset(offsetInSeconds);
}

void handleSleep() {
  timeClient.update();

  ulong now = millis();
  long elapsed = (now - lastSleepCheck);
  if (elapsed > SLEEP_CHECK_PERIOD_MS) {
    lastSleepCheck = now;
    bool shouldPrintRemainingTime = ((elapsed/1000) % MINUTES_TO_PRINT_SLEEP_INFO) == 0;
    handleAutoSleep();
  }
}

void handleAutoSleep() {
  return handleAutoSleep(false);
}

void handleAutoSleep(bool print) {
  if (!settings.autoSleep.enabled) {
    return;
  }

  bool isAllOff = isAllLightsOff();

  if (!isInactive) {
    if (isAllOff) {
      isInactive = true;
      inactiveTimeStart = millis();
      return;
    }
  } else if (!isAllOff) {
    isInactive = false;
    inactiveTimeStart = 0;
    return;
  }

  // check for how long it has been inactive
  ushort period = settings.autoSleep.period;
  int inactiveTimeInMinutes = (millis() - inactiveTimeStart) / 60000; // in minutes
  if (inactiveTimeInMinutes < period) {
    if (print) Serial.println("Auto sleep enabled. Will try to sleep in " + String(period - inactiveTimeInMinutes) + " minutes");
    return; // nothing to do yet
  }

  // we want to sleep - check if we are during active time
  DayOfWeek activeDays = settings.autoSleep.activeDaysOfWeek;
  byte hstart = settings.autoSleep.activeHourStart;
  byte hend = settings.autoSleep.activeHourEnd;

  DayOfWeek dcur = getDayOfWeek(timeClient.getEpochTime());
  int hcur = timeClient.getHours();

  // check for active day and active hours
  bool isActiveDay = (dcur & activeDays) > 0;
  if (isActiveDay) {
    bool isActiveHour = (hstart < hend) ? 
      (hcur >= hstart && hcur < hend) :
      (hcur >= hstart || hcur < hend);
    if (isActiveHour) {
      if (print) Serial.println("Auto sleep enabled. Prevented sleep due to active hours");
      return; // do nothing
    }
  }

  // determine for how long we can sleep based on active days and active hours
  // (we only need to look 1h in advance, as this is the max sleep time) 
  int mcur = timeClient.getMinutes();
  DayOfWeek dnext = getDayOfWeek(timeClient.getEpochTime(), 1);
  bool isNextDayActive = (dnext & activeDays) > 0;
  ulong sleepTimeInMinutes = 60;
  if (hcur >= 23 && mcur > 0) {
    sleepTimeInMinutes = (60-mcur) + (isNextDayActive ? hstart*60 : MAX_DEEP_SLEEP_IN_MINUTES);
  } else {
    sleepTimeInMinutes = ((24-hcur)*60)-mcur;
  }

  turnAllLightsOff();

  deepSleep(sleepTimeInMinutes);
}

void deepSleep(ulong sleepTimeInMinutes) {
  ulong validSleepTimeInMinutes = std::min(sleepTimeInMinutes, (ulong)MAX_DEEP_SLEEP_IN_MINUTES);
  Serial.println("Entering deep sleep for " + String(validSleepTimeInMinutes) + " minutes");

  digitalWrite(D0, LOW);
  ulong sleepTimeInMicroSeconds = validSleepTimeInMinutes*60*1000*1000; // in microseconds
  ESP.deepSleep(sleepTimeInMicroSeconds);
}

void lightSleep() {
#if defined(ESP8266)
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
#else
  Serial.println("Light sleep not configured/supported");
#endif
}


bool isWakingFromDeepSleep() {
#if defined(ESP8266)
  return ESP.getResetInfoPtr()->reason == REASON_DEEP_SLEEP_AWAKE;
#elif defined(ESP32)
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup caused by external signal using RTC_IO");
      return true;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      return true;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup caused by timer");
      return true;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Wakeup caused by touchpad");
      return true;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("Wakeup caused by ULP program");
      return true;
    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason);
      return false;
  }
#endif
}
