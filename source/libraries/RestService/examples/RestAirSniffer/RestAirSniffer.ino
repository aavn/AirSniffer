#include "Sniffer_Rest_Property.h"
#include "Sniffer_Rest_Util.h"
#include "Sniffer_Data_Util.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <SPI.h>


Environment envData;
#define sender "nJn1GBek70"
#define latitude "10.833787"
#define longitude "106.60434"

#define TEMP_SENSOR "DHT11"
#define PM_SENSOR "Nova fitness"


const char* ssid     = "your_wifi";
const char* password = "your_password";

void setup() {
  //setup wifi

  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  
  // put your setup code here, to run once:
   envData.novaPm25 = 10.0;
   envData.novaPm10 = 20.0;
   envData.temperature = 25.0;
   envData.humidity = 45.0;
   printData(envData);
   delay(5000);
  
   
   
}

void loop() {
  // put your main code here, to run repeatedly:
  if(isValidAirData(envData)){
    Serial.println("Data is  valid");  
  //  printData(envData);

    RestProperty restProperty;
    restProperty.sender_pro= sender;
    restProperty.latitude_pro= latitude;
    restProperty.longitude_pro= longitude;
    restProperty.TEMP_SENSOR_pro= TEMP_SENSOR;
    restProperty.PM_SENSOR_pro= PM_SENSOR;
	  restProperty.mac_str_pro = "FB-E3-DB-AB-5F-01";
    bool ok = saveData(envData,restProperty);
    if(ok){
      Serial.println("Saved on server");
    }else{
      Serial.println("Error calling WS");
    }
    
  }
  else{
   Serial.println("Data is not valid");  
  }
  delay(20000);

}