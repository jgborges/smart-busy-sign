#include "time.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void setupTime() {
    int offset = settings.tzOffsetInMinutes * 60; // in seconds
    timeClient.begin();
    timeClient.setTimeOffset(offset);
    timeClient.setUpdateInterval(3600000); // 1h
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

