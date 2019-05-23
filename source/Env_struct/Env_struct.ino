#include "SnifferTime.h"
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <Sniffer_Data_Util.h>
#include <TimeLib.h>
#include <Sniffer_Smart_Config.h>

#include <Sniffer_Wifi_Util.h>

#define ERR_PIN D4

BulkData bulkData;
HubConfig hubConfig;
Environment envData;
#define BULK_INDEX sizeof(hubConfig)
void setup() {
    Serial.begin(115200);
  Serial.println();

  // put your setup code here, to run once:
  initSnifferConfig(&hubConfig);
  initTestConfig();
  performSyncTime();
  Serial.println("Mocking data from the past");
  Serial.println("Loading bulk data");
  loadBulkData(&bulkData,BULK_INDEX);
  printBulkData(&bulkData);
  Serial.println("Filling more data");
  unsigned long curTime = now();
  for(int i = 0; i < 12; i++){
    envData.time = curTime;
    curTime -= 600;
    addBulkData(&bulkData, &envData);
  }
  
  printBulkData(&bulkData);
  
  saveBulkData(&bulkData,BULK_INDEX);
  
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
/*void saveBulkData(BulkData * bulk){
  int index = sizeof (hubConfig);
  EEPROM.put(index,*bulk);
  EEPROM.commit();
  delay(50);
}
void loadBulkData(BulkData * bulk){
  int index = sizeof (hubConfig);
  EEPROM.get(index,*bulk);
  
  if (bulk->bulkCount > BULK_CAPACITY || bulk->bulkCount < 0){
    bulk->bulkCount = 0;
    bulk->pointer = 0;
  }
}*/

void performSyncTime(){
  if (WiFi.status() != WL_CONNECTED) {
    
    connectWifi(&hubConfig, ERR_PIN);
  }
  syncSystemTime();
}
