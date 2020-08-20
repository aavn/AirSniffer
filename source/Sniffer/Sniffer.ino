#define SNIFFER_TEST false //true: running on test server, false: running on production server

#if SNIFFER_TEST
  #include <SnifferOTA_staging.h>
  #include <Sniffer_Rest_Util_staging.h>
#else 
  
  #include <SnifferOTA.h>
  #include <Sniffer_Rest_Util.h>
#endif  

#include <dht.h>
#include <SnifferTime.h>
#include <Sniffer_Smart_Config.h>
#include <Sniffer_Wifi_Util.h>
//#include <Sniffer_Rest_Constant.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <EEPROM.h>

#include "properties.h"
#include <SoftwareSerial.h>
#include <WinsenZE03.h>

SnifferDustSensor dustSensor;
Environment envData;
RestProperty restProperty ;
HubConfig hubConfig;
dht DHT;
#define BULK_INDEX sizeof(hubConfig)
int btnVoltage, refVoltage = 0;
bool led_status = false;
unsigned long lastCall;
unsigned long lastReadCall;
bool isPowerOn = true;
bool timeSynced = false;
#define LED_NORMAL 0
#define LED_WIFI_ERROR 1
#define LED_SAVE_DATA_ERROR 2
#define LED_READ_SENSOR_ERROR 3
int blink_type = LED_NORMAL;

unsigned long lastOTACheck=0;
BulkData bulkData;

void performOTA();

//SoftwareSerial gtSerial; // Arduino RX, Arduino TX
WinsenZE03 ozoneSensor;


void setup() {
  delay (200);
  
 
  Serial.begin(115200);
  Serial.println();
  Serial.println("Sniffer started");
  Serial.print("Sniffer version: ");
  Serial.println(VERSION);
  pinMode(CONFIG_BTN, INPUT_PULLUP);
  pinMode(ERR_PIN, OUTPUT);
///  pinMode(REF_PIN, INPUT);
  pinMode(DONE_PIN, OUTPUT);
  digitalWrite(DONE_PIN, LOW);
  attachInterrupt(CONFIG_BTN, highInterrupt, FALLING);

  initSnifferConfig(&hubConfig);
  
  printConfig(&hubConfig);
  
  if (isConfigMode(hubConfig.mode)) {
    setupAP(&hubConfig);

  } else {
    performOTA();
    
    performSyncTime();
    printDateTime();
    Serial.println("Loading bulk data");
    loadBulkData(&bulkData,BULK_INDEX);
    updateTimestamp(&bulkData);
    printBulkData(&bulkData);
    SoftwareSerial * gtSerial = new SoftwareSerial(OZONE_RX,OZONE_TX);
    gtSerial->begin(9600);
    ozoneSensor.begin(gtSerial, O3);
    ozoneSensor.setAs(QA);
    delay (200);
    dustSensor.begin(NOVA_RX, NOVA_TX);
  }
  lastCall = millis();
  lastReadCall = millis();

}
void fastBlinkForConfiguration() {
  if ((millis() - lastCall) > 300) {
    led_status = !led_status;
    lastCall = millis();
  }
  //   showStatus(led_status);
  if (led_status)
    turnLedOn();
  else
    turnLedOff();
}
void loop() {
  if (needRestart()) {
    ESP.restart();
  } else {
    if (isConfigMode(hubConfig.mode)) {
      handleSmartConfigClient();
      fastBlinkForConfiguration();
    } else {
      
      if (((millis() - lastReadCall) > 600000) || isPowerOn || (millis() < lastReadCall)) {
        performOTA();
        isPowerOn = false;
        lastReadCall = millis();
        turnLedOn();
        //for testing purpose
        readSnifferData();
        
        if(!isSystemTimeSynced()){
          performSyncTime();
          printDateTime();
          updateTimestamp(&bulkData);
        }
        
        envData.time = now();
        
        if (isValidAirData(envData)) {
          turnLedOn();
          if (WiFi.status() != WL_CONNECTED) {
            connectWifi(&hubConfig, ERR_PIN);
          }

          if (WiFi.status() == WL_CONNECTED) {
			      turnLedOn();
            readRestfulConfig();
            #if SNIFFER_TEST
            bool ok = saveData_staging(&envData, &restProperty);
            #else
            bool ok = saveData(&envData, &restProperty);
            
            #endif
            Serial.print("Save data:");
            Serial.println(ok);
            
            
            if (ok) {
              blink_type = LED_NORMAL; //everything ok, no blink
              sendBulkData();
            } else {
              addBulkData(&bulkData, &envData);
              saveBulkData(&bulkData,BULK_INDEX);
              blink_type = LED_SAVE_DATA_ERROR;
            }
            //WiFi.disconnect();
          } else {
            //save data for later 
            addBulkData(&bulkData, &envData);
            saveBulkData(&bulkData,BULK_INDEX);
            blink_type = LED_WIFI_ERROR;
          }
          Serial.println();
        } else {
          blink_type = LED_READ_SENSOR_ERROR;
        }
      }
      showSnifferStatus();
      for (int i=0; i<100; i++) {
        digitalWrite(DONE_PIN, HIGH);
        delay(5);
        digitalWrite(DONE_PIN, LOW);
        delay(5);
      }
    }
  } //not restart
} //loop

void readRestfulConfig() {
  
  restProperty.sender_pro = hubConfig.code;
  restProperty.latitude_pro = hubConfig.latitude;
  restProperty.longitude_pro = hubConfig.longitude;
  restProperty.TEMP_SENSOR_pro = TEMP_SENSOR;
  restProperty.PM_SENSOR_pro = PM_SENSOR;
  restProperty.mac_str_pro = hubConfig.macStr;
  restProperty.OZONE_SENSOR_pro = OZONE_SENSOR;
  restProperty.version_pro = VERSION;
}

void showSnifferStatus() {

  if (blink_type == LED_WIFI_ERROR) { //Wifi connection error
    turnLedOn();
    delay(1000);
    turnLedOff();
    delay(1000);
  } else {
    if (blink_type == LED_READ_SENSOR_ERROR)
      showSensorReadError();
    else if (blink_type == LED_SAVE_DATA_ERROR)
      showSaveDataError();
    delay(2000);
  }
}
void showSensorReadError() {
  for (int i = 0; i < LED_READ_SENSOR_ERROR; i++) {
    turnLedOn();
    delay(150);
    turnLedOff();
    delay(150);
  }
}

void showSaveDataError() {
  for (int i = 0; i < LED_WIFI_ERROR; i++) {
    turnLedOn();
    delay(150);
    turnLedOff();
    delay(150);
  }
}

ICACHE_RAM_ATTR void  highInterrupt() {
  btnVoltage = analogRead(CONFIG_BTN);
//  refVoltage = analogRead(REF_PIN);
  Serial.print("BTN: ");
  Serial.println(btnVoltage);
/*  Serial.print(" REF: ");
  Serial.println(refVoltage);*/
  if (btnVoltage == refVoltage) {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    prepareSmartConfig(&hubConfig);
    delay(5000);
    ESP.restart();
  }
}
void readSnifferData() {

  Serial.println("Reading PM Data");
  bool ok = dustSensor.readDustData();
  Serial.println("Ending read PM Data");
  envData.novaPm25 = dustSensor.getPM25();
  envData.novaPm10 = dustSensor.getPM10();

  envData.ozone = ozoneSensor.readManual();
  Serial.print("OZONE VALUE:");
  Serial.println(envData.ozone);
  int attemp = 0;
  while ((envData.ozone <= 0 || envData.ozone >= 10000) && attemp++ < 3) { //try 3 time read
    Serial.println("Ozone sensor doesn't work properly");
    envData.ozone = ozoneSensor.readManual();
    Serial.print("OZONE VALUE:");
    Serial.println(envData.ozone);
    delay(5000);
  }
  
  readTemperatureAndHumidity();
  attemp = 0;
  while ((envData.temperature <= 0 || envData.humidity <= 0) && attemp++ < 3) { //try 3 time read
    Serial.println("DHT11 sensor doesn't work properly");
    readTemperatureAndHumidity();
    delay(5000);
  }
  printData(&envData);
  delay(1000);
  Serial.println("Done reading data");
}

void readTemperatureAndHumidity() {

  Serial.print("DHT11, \t");
  int chk = DHT.read11(DHT11_PIN);
  switch (chk)
  {
    case DHTLIB_OK:
      Serial.print("OK,\t");
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.print("Checksum error,\t");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.print("Time out error,\t");
      break;
    default:
      Serial.print("Unknown error,\t");
      break;
  }


  envData.temperature = DHT.temperature;
  envData.humidity = DHT.humidity;
  // DISPLAY DATA
  Serial.print(DHT.humidity, 1);
  Serial.print(",\t");
  Serial.println(DHT.temperature, 1);

}
void turnLedOn() {
  digitalWrite(ERR_PIN, HIGH);
}
void turnLedOff() {
  digitalWrite(ERR_PIN, LOW);
}
void performOTA(){
  
    if(isOTA(hubConfig.ota) ){
    //ignore OTA if last check was less than 24h
      if(lastOTACheck > 0 && millis() - lastOTACheck < ONE_DAY){
        return;
      }
      
      if (WiFi.status() != WL_CONNECTED) {
    
        connectWifi(&hubConfig, ERR_PIN);
      }
      if (WiFi.status() == WL_CONNECTED) {
        #if SNIFFER_TEST 
        if(checkAndUpdate_staging(VERSION)== HTTP_UPDATE_OK)
        
        #else
        if(checkAndUpdate(VERSION)== HTTP_UPDATE_OK)
        #endif  
        {
          Serial.println("Hoooray! I got new version! Restarting myself");
          Serial.flush();
          delay(100);
          ESP.restart();
        }else{
          
          lastOTACheck = millis();
          Serial.println("No update, check again after 24h");
        }
      }else{
        Serial.println("No wifi connection");
      }
    }else{
      Serial.println("My owner want to handle upgrade manually!");
    }
  
}

void updateTimestamp(BulkData * bulk){
    for (int i = 0; i < bulk->bulkCount;i++){
      updateTimestamp(&bulk->data[i].time);
    }
}
void performSyncTime(){
  if (WiFi.status() != WL_CONNECTED) {
    
    connectWifi(&hubConfig, ERR_PIN);
  }
  syncSystemTime();
}
void sendBulkData(){
  Serial.print("Send bulk data");
  bool sent = false;
  for(int i = bulkData.bulkCount - 1; i >= 0; i--){
    //dont flood the server
    delay(10000);
    #if SNIFFER_TEST
    sent = saveData_staging(&bulkData.data[i], &restProperty);
    #else
    sent = saveData(&bulkData.data[i], &restProperty);
    #endif
    Serial.print("Save data:");
    Serial.println(sent);
    if (sent){
      //update bulk data record
      if(bulkData.pointer == i){
        bulkData.pointer -= 1;
      }
      bulkData.bulkCount -= 1;
    }else{
      //no need to go further
      break;
    }
  }
  saveBulkData(&bulkData,BULK_INDEX);
  Serial.println("Finish sending bulk data!!");

}
