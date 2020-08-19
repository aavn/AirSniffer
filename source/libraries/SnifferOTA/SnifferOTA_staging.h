#ifndef _SNIFFER_OTA_STAGING_H
#define _SNIFFER_OTA_STAGING_H
#include <SPI.h>
#include <ESP8266httpUpdate.h>

#define SERVER_ADDRESS_STAGING "raw.githubusercontent.com"
#define RELEASE_URL_STAGING "/aavn/AirSniffer/master/staging/package.json"
#define DOWNLOAD_URL_STAGING "https://raw.githubusercontent.com/aavn/AirSniffer/master/staging/"

	

HTTPUpdateResult checkAndUpdate_staging(String currentVersion);


#endif
