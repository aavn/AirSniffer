#ifndef _SNIFFER_OTA_H
#define _SNIFFER_OTA_H
#include <SPI.h>
#include <ESP8266httpUpdate.h>


#define SERVER_ADDRESS "raw.githubusercontent.com"
#define RELEASE_URL "/aavn/AirSniffer/master/releases/package.json"
#define SERVER_FINGER "CC:AA:48:48:66:46:0E:91:53:2C:9C:7C:23:2A:B1:74:4D:29:9D:33"
#define DOWNLOAD_URL "https://raw.githubusercontent.com/aavn/AirSniffer/master/releases/"
#define RELEASE_VERSION_KEY "version"
#define BIN_NM_KEY "file"
	

HTTPUpdateResult checkAndUpdate(String currentVersion);


#endif
