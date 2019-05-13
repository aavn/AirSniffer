
#define SNIFFER_TEST true //true: running on test server, false: running on production server

#if SNIFFER_TEST
  #include <SnifferOTA_staging.h>
  #include <Sniffer_Rest_Util_staging.h>
#else 
  
  #include <SnifferOTA.h>
  #include <Sniffer_Rest_Util.h>
#endif  

#include <dht.h>

#include <Sniffer_Smart_Config.h>
#include <Sniffer_Wifi_Util.h>
//#include <Sniffer_Rest_Constant.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <EEPROM.h>

#include "properties.h"
unsigned int localPort = 2390;      // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
    Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "0.asia.pool.ntp.org";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
SnifferDustSensor dustSensor;
Environment envData;

HubConfig hubConfig;
dht DHT;

//char data[50] = {0};

//int i = 0;
int btnVoltage, refVoltage;
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
#define BULK_CAPACITY 12
Environment bulkData[BULK_CAPACITY];
int bulkCount = 0;

void performOTA();
void syncTime();
void sendNTPpacket(IPAddress& address);
void syncTime();
void printDateTime();

void setup() {

  Serial.begin(115200);
  Serial.println();
  Serial.println("Sniffer started");
  Serial.print("Sniffer version: ");
  Serial.println(VERSION);
  pinMode(CONFIG_BTN, INPUT_PULLUP);
  pinMode(ERR_PIN, OUTPUT);
  pinMode(REF_PIN, OUTPUT);
  attachInterrupt(CONFIG_BTN, highInterrupt, FALLING);

  initSnifferConfig(&hubConfig);
  printConfig(&hubConfig);
  
  if (isConfigMode(hubConfig.mode)) {
    setupAP(&hubConfig);

  } else {
    performOTA();
    dustSensor.begin(NOVA_RX, NOVA_TX);
    syncTime();
    printDateTime();
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
      performOTA();
      if (((millis() - lastReadCall) > 600000) || isPowerOn || (millis() < lastReadCall)) {
        isPowerOn = false;
        lastReadCall = millis();
        turnLedOn();
        readSnifferData();
        if(!timeSynced){
          syncTime();
          printDateTime();
        }
        if(timeSynced){
          envData.time = now();
        }else{
          envData.time = 0;
        }
        //for testing purpose
        /*envData.novaPm25 = 10.0;
         envData.novaPm10 = 20.0;
         envData.temperature = 25.0;
         envData.humidity = 45.0;
         */
        if (isValidAirData(envData)) {
          turnLedOn();
          if (WiFi.status() != WL_CONNECTED) {
            connectWifi(&hubConfig, ERR_PIN);
          }

          if (WiFi.status() == WL_CONNECTED) {

            RestProperty restProperty = readRestfulConfig();
            #if SNIFFER_TEST
            bool ok = saveData_staging(&envData, &restProperty);
            #else
            bool ok = saveData(&envData, &restProperty);
            
            #endif
            Serial.print("Save data:");
            Serial.println(ok);
            WiFi.disconnect();
            if (ok) {
              blink_type = LED_NORMAL; //everything ok, no blink

            } else {
              blink_type = LED_SAVE_DATA_ERROR;
            }

          } else {
            blink_type = LED_WIFI_ERROR;
          }
          Serial.println();
        } else {
          blink_type = LED_READ_SENSOR_ERROR;
        }
      }

      showSnifferStatus();
    }
  } //not restart
} //loop

RestProperty readRestfulConfig() {
  RestProperty restProperty;
  restProperty.sender_pro = hubConfig.code;
  restProperty.latitude_pro = hubConfig.latitude;
  restProperty.longitude_pro = hubConfig.longitude;
  restProperty.TEMP_SENSOR_pro = TEMP_SENSOR;
  restProperty.PM_SENSOR_pro = PM_SENSOR;
  restProperty.mac_str_pro = hubConfig.macStr;
  
  return restProperty;
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

void highInterrupt() {
  btnVoltage = analogRead(CONFIG_BTN);
  refVoltage = analogRead(REF_PIN);
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

  readTemperatureAndHumidity();
  int attemp = 0;
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
void syncTime(){
  if (WiFi.status() != WL_CONNECTED) {
    
    connectWifi(&hubConfig, ERR_PIN);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Starting UDP");
    udp.begin(localPort);
    Serial.print("Local port: ");
    Serial.println(udp.localPort());
    //get a random server from the pool
    WiFi.hostByName(ntpServerName, timeServerIP);
  
    sendNTPpacket(timeServerIP); // send an NTP packet to a time server
    // wait to see if a reply is available
    delay(3000);
  
    int cb = udp.parsePacket();
    if (!cb) {
      Serial.println("no packet yet");
    } else {
      Serial.print("packet received, length=");
      Serial.println(cb);
      // We've received a packet, read the data from it
      udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  
      //the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, esxtract the two words:
  
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      Serial.print("Seconds since Jan 1 1900 = ");
      Serial.println(secsSince1900);
  
      // now convert NTP time into everyday time:
      Serial.print("Unix time = ");
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;
      // subtract seventy years:
      unsigned long epoch = secsSince1900 - seventyYears;
      // print Unix time:
      Serial.println(epoch);
  
  
      // print the hour, minute and second:
      Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
      Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
      Serial.print(':');
      if (((epoch % 3600) / 60) < 10) {
        // In the first 10 minutes of each hour, we'll want a leading '0'
        Serial.print('0');
      }
      Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
      Serial.print(':');
      if ((epoch % 60) < 10) {
        // In the first 10 seconds of each minute, we'll want a leading '0'
        Serial.print('0');
      }
      Serial.println(epoch % 60); // print the second
      setTime(epoch);
      timeSynced = true;
    }
  }else{
    Serial.print("Cannot connect time server!");
  }
}
// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
  Serial.println("Finish sending NTP packet...");
}


void printDateTime(){
  Serial.print(year());
  Serial.print('-');
  Serial.print(month());
  Serial.print('-');
  Serial.print(day());
  Serial.print('T');
  Serial.print(hour());
  Serial.print(':');
  Serial.print(minute());
  Serial.print(':');
  Serial.print(second());
  Serial.println(".000Z");
  Serial.println(now());
}
void readBulkData(){
  
}
void saveBulkData(Environment * envRecord){
  
}
