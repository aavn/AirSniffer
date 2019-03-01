#include <SPI.h>
#include <dht.h>
#include <Sniffer_Smart_Config.h>
#include <Sniffer_Wifi_Util.h>
#include <Sniffer_Dust_Sensor.h>
#include <Sniffer_Data_Util.h>
#include <Sniffer_Rest_Util.h>
//for portable IDE
/*#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>*/

#include "properties.h"

SnifferDustSensor dustSensor;
Environment envData;

HubConfig hubConfig;
dht DHT;

char data[50]= {0};

int i=0;
int btnVoltage, refVoltage;

void highInterrupt();
void showStatus(bool ok);
bool led_status = false;
unsigned long lastCall;
unsigned long lastReadCall;
bool restarted = true;
int blink_type = 1; //0: no blink; 1: blink; 2: flash twice; 3: flash three

void setup(){
  //ESP.eraseConfig();
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  Serial.println();
  Serial.println();
  Serial.println("Hub started");
  pinMode(CONFIG_BTN, INPUT_PULLUP);
  pinMode(ERR_PIN, OUTPUT);
  pinMode(REF_PIN, OUTPUT);
  //pinMode(OK_PIN, OUTPUT);
 
  attachInterrupt(CONFIG_BTN, highInterrupt, FALLING);

  initSnifferConfig(&hubConfig);
  if(isConfigMode(&hubConfig)){
    setupAP(&hubConfig);
    
  }else{
//    connectWifi(&hubConfig, ERR_PIN);

    //start ok
    dustSensor.begin(NOVA_RX,NOVA_TX);
    Serial.print("LIBRARY VERSION: ");
    Serial.println(DHT_LIB_VERSION);
  }
  lastCall = millis();  
  lastReadCall = millis();
  
}

void loop(){
  //Serial.print("Mode: " );
  //Serial.println(hubConfig.mode);
  if(needRestart()){
    ESP.restart();
  }else{
    if(isConfigMode(&hubConfig)){
      handleSmartConfigClient();
      delay(6000);
      if ((millis()-lastCall) > 300) {
        led_status = !led_status;
        lastCall = millis();
      }
      showStatus(led_status);
    }else{
      
      if (((millis()-lastReadCall) > 600000) || restarted || (millis()<lastReadCall)) {
        restarted = false;
        lastReadCall = millis();
        showStatus(true);
        readSnifferData();
        if (isValidAirData(envData)) {
          showStatus(true);
          if(WiFi.status() != WL_CONNECTED){
            connectWifi(&hubConfig, ERR_PIN);
          }
          
          if(WiFi.status() == WL_CONNECTED){
            RestProperty restProperty;
            restProperty.sender_pro= hubConfig.code;
            restProperty.latitude_pro= hubConfig.latitude;
            restProperty.longitude_pro= hubConfig.longitude;
            restProperty.TEMP_SENSOR_pro= TEMP_SENSOR;
            restProperty.PM_SENSOR_pro= PM_SENSOR;
            restProperty.mac_str_pro = hubConfig.macStr;
            bool ok = saveData(envData,restProperty);
            Serial.print("Save data:");
            Serial.println(ok);
            WiFi.disconnect();
            if (ok) { 
              blink_type = 0; //everything ok, no blink
              showStatus(ok);
              
            } else {
              blink_type = 2;
            }
            
          } else{
//            showStatus(false);
            blink_type = 1;
          }
          Serial.println();
        }else{
//          showStatus(false);
          blink_type = 3;
        }
      }
      if (blink_type==1) { //Wifi connection error
        showStatus(true);
        delay(1000);
        showStatus(false);
        delay(1000);
      } else {
        flash(blink_type);
        delay(2000);
      }
//      delay(600000);
    }
  }
}

void flash(int numberOfFlash) {
  for (int i=0; i< numberOfFlash; i++) {
    showStatus(true);
    delay(150);
    showStatus(false);
    delay(150);
  }
}

void highInterrupt(){
  btnVoltage = analogRead(CONFIG_BTN);
  refVoltage = analogRead(REF_PIN);
  if(btnVoltage == refVoltage){
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    prepareSmartConfig(&hubConfig);
    ESP.restart();
  }
}
void readSnifferData()
{
    
    //delay(5000);
    //read dust sensor value
    Serial.println("Reading PM Data");
    bool ok = dustSensor.readDustData();
    Serial.println("Ending read PM Data");
    //if(ok){
      envData.novaPm25 = dustSensor.getPM25();
      envData.novaPm10 = dustSensor.getPM10();
      
      Serial.print ("PM10:");
      Serial.println(dustSensor.getPM10());
      Serial.print ("PM2.5:");
      Serial.println(dustSensor.getPM25());
    
    //}
     
    //read 
    readTemperatureAndHumidity();
    int attemp = 0; 
    while((envData.temperature <= 0 || envData.humidity <= 0) && attemp++<3) { //try 3 time read
      Serial.println("DHT11 sensor doesn't work properly");
      readTemperatureAndHumidity();  
      delay(5000);
    }
    printData(envData);
    delay(1000);
    Serial.println("Done reading data");
       
   
}


//read temperature and humidity from DHT11
void readTemperatureAndHumidity(){
  
  // READ DATA
  Serial.print("DHT11, \t");
  int chk = DHT.read11(DHT11_PIN);
  bool readSucess = false;
  for (int i = 0; i < 5 && !readSucess; i++){
    switch (chk)
    {
      case DHTLIB_OK:  
      readSucess = true;
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
    delay (100);
  }

  envData.temperature = DHT.temperature;
  envData.humidity = DHT.humidity;
  // DISPLAY DATA
  Serial.print(DHT.humidity, 1);
  Serial.print(",\t");
  Serial.println(DHT.temperature, 1);
  
  }
void showStatus(bool ok){
  if(ok){
//    digitalWrite(OK_PIN,HIGH);
    digitalWrite(ERR_PIN,HIGH);
  }else{
//    digitalWrite(OK_PIN,LOW);
    digitalWrite(ERR_PIN,LOW);
  }
}
