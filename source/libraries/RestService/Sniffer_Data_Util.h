#ifndef _DATAUTIL_H
#define _DATAUTIL_H

#include <Sniffer_Rest_Property.h>
//macro
#define isValidAirData(env) (env.novaPm25>0)|| (env.novaPm10>0) || (env.temperature>0) || (env.humidity>0)
struct Environment{
  float novaPm25;
  float novaPm10;
  float temperature;
  float humidity;

};

//void convertAirData(String dataStr);
void formatAAVNData(char * dataStr, Environment * envirData, RestProperty * restProperty);
void printData(Environment * envirData);

#endif
