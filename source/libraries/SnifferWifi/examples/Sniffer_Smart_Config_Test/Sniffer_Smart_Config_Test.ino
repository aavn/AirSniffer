/************************************************************************************
 * Connect D0 to GND as the voltage reference PIN 
 * (to avoid accidentally start smart config because of noise voltage)
 * 
 * Smart Config process will:
 * - start the SNIFFER access point (without password)
 * - user connect to SNIFFER using mobile or laptop
 * - user open a web browser, go to 192.168.4.1
 * - user select a wifi network
 * - user enter wifi password and press Save
 * - wifi SSID and password will be save to EEPROM
 * - ESP restarts, loads wifi SSID & password from EEPROM, connects to wifi network
 * 
 * The sketch will try to load wifi configuration from EEPROM.
 * If there is no configuration found, then it automatically start Smart Config process.
 * If you want to manually start Smart Config, connect a push button between GND and D4, press the button.
 ************************************************************************************/
#include <Sniffer_Wifi_Util.h>

#define WIFI_ERR_PIN 14
#define CONFIG_BTN 2
#define REF_PIN 16


//#define HAS_LCD 1

HubConfig hubConfig;
int btnVoltage, refVoltage;
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Hub started");

  pinMode(CONFIG_BTN, INPUT_PULLUP);
  pinMode(WIFI_ERR_PIN, OUTPUT);
  pinMode(REF_PIN, OUTPUT);

  digitalWrite(WIFI_ERR_PIN,HIGH);
  initSnifferConfig(&hubConfig);

  attachInterrupt(CONFIG_BTN, highInterrupt, FALLING);
  if(isConfigMode(&hubConfig)){
    setupAP(&hubConfig);
    
  }else{
    connectWifi(&hubConfig,WIFI_ERR_PIN);
  }
}

void loop() {
  if(needRestart()){
    
    #if defined(HAS_LCD)
      Serial.println("Please restart your Sniffer");
      delay(1000);
    #else
      ESP.restart();
    #endif
  }else{
    if(isConfigMode(&hubConfig)){
      
      handleSmartConfigClient();
    }else{
      delay (50000);
    }
  }
}

void highInterrupt(){
  btnVoltage = analogRead(CONFIG_BTN);
  refVoltage = analogRead(REF_PIN);
  if(btnVoltage == refVoltage){
    prepareSmartConfig(&hubConfig);
    ESP.restart();
  }
}
