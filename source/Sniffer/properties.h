#ifndef _PROPERTIES_H
#define _PROPERTIES_H

#include <dht.h>
#include <Sniffer_Dust_Sensor.h>

#if SNIFFER_TEST
  #define VERSION "0.19.23_rc" //release build
#else 
  #define VERSION "0.19.23"
#endif

#define TEMP_SENSOR "DHT11"
#define PM_SENSOR "Nova fitness"

#define ONE_DAY 86400000 //ms of 24h

//extern ESP8266WebServer server;
//extern bool wifiStatus;
extern dht DHT;

#define DHT11_PIN D3 //GPO05 
#define CONFIG_BTN D4 //GPO02
#define ERR_PIN D2  //GPO14
//#define OK_PIN D2  //GPO04
#define NOVA_RX D7  //GPO13
#define NOVA_TX D6  //GPO12
#define REF_PIN D0  //GPO16
#define DONE_PIN D1 


//#define SENSOR_ERR 3

#endif
