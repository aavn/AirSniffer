#ifndef _PROPERTIES_H
#define _PROPERTIES_H

#include <dht.h>
#include <Sniffer_Dust_Sensor.h>

#if SNIFFER_TEST
  #define VERSION "0.20.04_rc" //release build
#else 
  #define VERSION "0.20.03"
#endif

#define TEMP_SENSOR "DHT11"
#define PM_SENSOR "Nova fitness"
#define OZONE_SENSOR  "Winsen Z5"

#define ONE_DAY 86400000 //ms of 24h

//extern ESP8266WebServer server;
//extern bool wifiStatus;
extern dht DHT;

//input pins
#define DHT11_PIN D3 //GPO05 
#define CONFIG_BTN D4 //GPO02
#define NOVA_RX D7  //GPO13
#define NOVA_TX D6  //GPO12
#define OZONE_RX D5
#define OZONE_TX D8//D0

//output pins
#define ERR_PIN D2  //GPO14
#define DONE_PIN D1 


#endif
