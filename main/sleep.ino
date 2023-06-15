#include "time.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void setupTime() {
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

