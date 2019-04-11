#ifndef _SNIFFER_OTA_STAGING_H
#define _SNIFFER_OTA_STAGING_H
#include <SPI.h>
#include <ESP8266httpUpdate.h>

#define SERVER_ADDRESS_STAGING "raw.githubusercontent.com"
#define RELEASE_URL_STAGING "/aavn/AirSniffer/master/staging/package.json"
#define SERVER_FINGER_STAGING "CC:AA:48:48:66:46:0E:91:53:2C:9C:7C:23:2A:B1:74:4D:29:9D:33"
#define DOWNLOAD_URL_STAGING "https://raw.githubusercontent.com/aavn/AirSniffer/master/staging/"

	

HTTPUpdateResult checkAndUpdate_staging(String currentVersion);


#endif
