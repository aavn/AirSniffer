#ifndef _PROPERTIES_H
#define _PROPERTIES_H
#include "Arduino.h"
#include <SoftwareSerial.h>



#define STRING_SIZE 80
#define SAMPLES 10
#define SLEEP_PERIOD 15 //15 minutes sleep
#define SENSOR_ERR 3
#define LED_SIGNAL 13

struct Environment{
  float novaPm25;
  float novaPm10;
  float temperature;
  float humidity;
};

extern Environment envData;
extern SoftwareSerial novaSerial;

#endif
