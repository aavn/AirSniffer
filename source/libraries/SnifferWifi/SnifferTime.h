#ifndef _SNIFFER_TIME_H
#define _SNIFFER_TIME_H
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Sniffer_Smart_Config.h>

#define ANCIENT_TIME 1970
//macro
void syncSystemTime();
void sendNTPpacket(IPAddress& address);
time_t getTimeGap();//in seconds
void printDateTime();
void updateTimestamp(time_t * timeSeconds);
bool isTimeSynced(time_t timeSeconds);
bool isSystemTimeSynced();
#endif
