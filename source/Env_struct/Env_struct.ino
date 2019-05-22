#include "SnifferTime.h"
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <Sniffer_Data_Util.h>
#include <TimeLib.h>
#include <Sniffer_Smart_Config.h>



BulkData bulkData;
HubConfig hubConfig;
void setup() {
    Serial.begin(115200);
  Serial.println();

  // put your setup code here, to run once:
  initSnifferConfig(&hubConfig);
  initTestConfig();
  
  
  unsigned long curTime = now();
  for(bulkData.pointer = 0;  bulkData.pointer < 5; bulkData.pointer++){
    bulkData.data[bulkData.pointer].time = curTime;
    curTime += 600;
    
  }
  bulkData.bulkCount = 5;
  printBulkData(&bulkData);
  bulkData.timeSynced = syncTime(&hubConfig);
  Serial.println("Updating timestamp");
  //curTime = now();
  for(bulkData.pointer = 0;  bulkData.pointer < bulkData.bulkCount; bulkData.pointer++){
    //bulkData.data[i].time = curTime;
    //curTime += 600;
    bulkData.data[bulkData.pointer].time += getTimeGap();
    curTime = bulkData.data[bulkData.pointer].time;
  }
  printBulkData(&bulkData);;
  Serial.println("Filling more data");
  //curTime = now();
  for(bulkData.pointer; bulkData.pointer < BULK_CAPACITY;bulkData.pointer++){
    bulkData.data[bulkData.pointer].time = curTime;
    curTime += 600;
    bulkData.bulkCount ++;
  }
  printBulkData(&bulkData);
  Serial.println("Filling more data");
  //curTime = now();
  if(bulkData.pointer <= BULK_CAPACITY){
    bulkData.pointer = 0;
  }
  for(bulkData.pointer; bulkData.pointer < 5;bulkData.pointer++){
    bulkData.data[bulkData.pointer].time = curTime;
    curTime += 600;
    
  }
  printBulkData(&bulkData);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(6000000);
}

void initTestConfig(){
  strcpy(hubConfig.ssid ,"IoT");
  strcpy(hubConfig.pwd ,"IoT@@@VN1@3");
  strcpy(hubConfig.code ,"9sTwEkrvhe");
  strcpy(hubConfig.latitude ,"10.8312");
  strcpy(hubConfig.longitude ,"106.6355");
  strcpy(hubConfig.macStr ,"5C-CF-7F-0C-3D-CD");
}
