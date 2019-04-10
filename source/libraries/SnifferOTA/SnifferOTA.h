#ifndef _SNIFFER_OTA_H
#define _SNIFFER_OTA_H
#include <SPI.h>
#include <ESP8266httpUpdate.h>

/*#ifdef SNIFFER_TEST
	#define SERVER_ADDRESS "raw.githubusercontent.com"
	#define RELEASE_URL "/aavn/AirSniffer/ota_mvp/staging/package.json"
	#define SERVER_FINGER "CC:AA:48:48:66:46:0E:91:53:2C:9C:7C:23:2A:B1:74:4D:29:9D:33"
	#define DOWNLOAD_URL "https://raw.githubusercontent.com/aavn/AirSniffer/ota_mvp/staging/"
	#define RELEASE_VERSION_KEY "version"
	#define BIN_NM_KEY "file"
#else
*/
	#define SERVER_ADDRESS "raw.githubusercontent.com"
	#define RELEASE_URL "/aavn/AirSniffer/ota_mvp/releases/package.json"
	#define SERVER_FINGER "CC:AA:48:48:66:46:0E:91:53:2C:9C:7C:23:2A:B1:74:4D:29:9D:33"
	#define DOWNLOAD_URL "https://raw.githubusercontent.com/aavn/AirSniffer/ota_mvp/releases/"
	#define RELEASE_VERSION_KEY "version"
	#define BIN_NM_KEY "file"
//#endif

HTTPUpdateResult checkAndUpdate(String currentVersion);


#endif
