#ifndef _SNIFFER_TIME_H
#define _SNIFFER_TIME_H
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Sniffer_Smart_Config.h>

bool syncTime(HubConfig* smartConfig);
void sendNTPpacket(IPAddress& address);
time_t getTimeGap();
#endif
