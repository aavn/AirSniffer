#include <Sniffer_Smart_Config.h>
#include <Sniffer_Wifi_Util.h>
#include <EEPROM.h>
HubConfig hubConfig;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Sniffer started");

  initSnifferConfig(&hubConfig);
  printConfig(&hubConfig);
}

void loop() {
  delay(5000);
}
