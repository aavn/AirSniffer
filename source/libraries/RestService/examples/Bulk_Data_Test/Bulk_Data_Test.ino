#include <SnifferTime.h>
#include <Sniffer_Data_Util.h>
#include <Sniffer_Smart_Config.h>
#include <Sniffer_Wifi_Util.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
/*
 * Seconds since Jan 1 1900 = 3780189255
 * Unix time = 1571200455
 * The UTC time is 4:34:15
 * 2019-10-16T4:34:15.000Z
 * 1571200455

 */
HubConfig hubConfig;
#define BULK_INDEX sizeof(hubConfig)
BulkData bulkData;
unsigned long epoch = 1571200455;
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Sniffer started");
  initSnifferConfig(&hubConfig);
  printConfig(&hubConfig);
  loadBulkData(&bulkData,BULK_INDEX);
  bulkData.data[0].time = now();
  printBulkData(&bulkData);
  //correct the time to 2019-10-16T4:34:15.000Z
  //setTime(epoch);
  performSyncTime();
  Serial.println(year());
  Serial.println(ANCIENT_TIME);
  Serial.println(getTimeGap());
  updateTimestamp(&bulkData);
  printBulkData(&bulkData);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(500);
}
void updateTimestamp(BulkData * bulk){
    for (int i = 0; i < bulk->bulkCount;i++){
      updateTimestamp(&bulk->data[i].time);
    }
}
void performSyncTime(){
  if (WiFi.status() != WL_CONNECTED) {
    
    connectWifi(&hubConfig, 13);
  }
  syncSystemTime();
}
