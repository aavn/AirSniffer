#include "Sniffer_Rest_Property.h"

#include "Sniffer_Data_Util.h"

#include <SPI.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <Sniffer_Smart_Config.h>
#include <Sniffer_Wifi_Util.h>

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
HubConfig hubConfig;
Environment envData;
#define ERR_PIN D2  //GPO14
#define sender "nJn1GBek70"
#define lat "10.833787"
#define longi "106.60434"

#define TEMP_SENSOR "DHT11"
#define PM_SENSOR "Nova fitness"


char data[600];
RestProperty restProperty;

void syncTime();
void sendNTPpacket(IPAddress& address);

void setup() {
  //setup wifi

  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  

  
  // put your setup code here, to run once:


  strcpy(hubConfig.ssid ,"IoT");
  strcpy(hubConfig.pwd ,"IoT@@@VN1@3");
  strcpy(hubConfig.code ,"9sTwEkrvhe");
  strcpy(hubConfig.latitude ,"10.8312");
  strcpy(hubConfig.longitude ,"106.6355");
  strcpy(hubConfig.macStr ,"5C-CF-7F-0C-3D-CD");

    syncTime();
    
   envData.novaPm25 = 10.0;
   envData.novaPm10 = 20.0;
   envData.temperature = 25.0;
   envData.humidity = 45.0;
   envData.time = now();
   printData(&envData);
   restProperty.sender_pro= sender;
    restProperty.latitude_pro= lat;
    restProperty.longitude_pro= longi;
    restProperty.TEMP_SENSOR_pro= TEMP_SENSOR;
    restProperty.PM_SENSOR_pro= PM_SENSOR;
    restProperty.mac_str_pro ="FB-E3-DB-AB-5F-01";
    //strcpy(restProperty.mac_str_pro ,"FB-E3-DB-AB-5F-01");
   delay(5000);
  
   formatAAVNData(data, &envData,&restProperty);
   Serial.print("Data: ");
  Serial.println(data);
}

void loop() {
  
  delay(20000);

}
void syncTime(){
  if (WiFi.status() != WL_CONNECTED) {
    
    connectWifi(&hubConfig, ERR_PIN);
  }
  if (WiFi.status() == WL_CONNECTED) {
    //get a random server from the pool
    WiFi.hostByName(ntpServerName, timeServerIP);
  
    sendNTPpacket(timeServerIP); // send an NTP packet to a time server
    // wait to see if a reply is available
    delay(1000);
  
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
}
