#include "Sniffer_Wifi_Util.h"
#include <EEPROM.h>

//private functions
boolean testWifi(int ERR_PIN);

/* Set these to your desired credentials. */
void connectWifi(HubConfig* smartConfig, int ERR_PIN){
  
      WiFi.mode(WIFI_STA);
      WiFi.begin(smartConfig->ssid, smartConfig->pwd);
      if(testWifi(ERR_PIN)){
        Serial.println("Wifi connected, running in normal mode");
      }else{
        Serial.println("Wifi not connected!");
      }
      
  Serial.print("Mode: " );
  Serial.println(smartConfig->mode);
  
}

boolean testWifi(int ERR_PIN) {
  int c = 0;
  bool status = false;
  Serial.println("Waiting for Wifi to connect");  
  while ( c < RETRY_LIM ) {
    if (WiFi.status() == WL_CONNECTED) { 
      break; 
    } 
    delay(1000);
	Serial.print("Wifi status: ");
    Serial.println(WiFi.status());    
	status = !status;
	if (status) 
		digitalWrite(ERR_PIN,HIGH);
	else
		digitalWrite(ERR_PIN,LOW);
    c++;
  }
  if (WiFi.status() == WL_CONNECTED) { 
    return true; 
  } else{
    Serial.println("Connect timed out, no internet");
    return false;
  }
  
} 

