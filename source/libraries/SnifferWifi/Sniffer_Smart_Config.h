#ifndef _SMARTCONFIG_H
#define _SMARTCONFIG_H
#include <ESP8266WiFi.h>

#define smartConfigSSID "SNIFFER"
#define httpPort 80

//macros
#define isOTA(ota_st)  (ota_st == 1)
#define isConfigMode(config_st) (config_st == 1)
typedef struct {
   uint8_t mode;
   uint8_t ota;
   char ssid[36];
   char pwd[50];
   char code[17];
   char latitude[12];
   char longitude[12];
   char macStr[18];   
} HubConfig;

void setupAP(HubConfig* oldConfig);
void storeConfig(HubConfig* smartConfig);
void loadConfig(HubConfig* smartConfig);
void handleSmartConfigClient(void);

void initSnifferConfig(HubConfig* smartConfig);
void prepareSmartConfig(HubConfig* smartConfig);
//just for testing
void printConfig(HubConfig* smartConfig);
bool needRestart();

void clearStoredConfig(void);
#endif
