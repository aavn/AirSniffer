#ifndef _SNIFFER_OTA_H
#define _SNIFFER_OTA_H
#include <SPI.h>
#include <ESP8266httpUpdate.h>

#define SERVER_ADDRESS "raw.githubusercontent.com"

#define RELEASE_URL "/aavn/AirSniffer/refactoring/releases/package.json"
#define GITHUB_FINGER "CC:AA:48:48:66:46:0E:91:53:2C:9C:7C:23:2A:B1:74:4D:29:9D:33"
//#define DOWNLOAD_URL "https://github.com/aavn/AirSniffer/releases/download/sprint_2019_09/Sniffer.ino.bin"
#define DOWNLOAD_URL "https://raw.githubusercontent.com/aavn/AirSniffer/refactoring/releases/Sniffer.bin"
#define RELEASE_VERSION_KEY "version"

HTTPUpdateResult checkAndUpdate(String currentVersion);


#endif
