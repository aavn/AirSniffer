/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>


#include <ESP8266HTTPClient.h>
#include "SnifferOTA.h"

#define USE_SERIAL Serial


//String gitHubFinger = "CC:AA:48:48:66:46:0E:91:53:2C:9C:7C:23:2A:B1:74:4D:29:9D:33";
bool ok = false;
void setup() {

  USE_SERIAL.begin(115200);
  // USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin("IoT", "IoT@@@VN1@3");
  int c = 0;
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) { 
      break; 
    } 
    delay(1000);
  Serial.print("Wifi status: ");
    Serial.println(WiFi.status());    
  
    c++;
  }
  if (WiFi.status() == WL_CONNECTED && !ok) {
    Serial.println("Getting data ");
    /*HTTPClient http;  //Declare an object of class HTTPClient
    http.begin("https://raw.githubusercontent.com/aavn/AirSniffer/refactoring/releases/package.json",GITHUB_FINGER);  //Specify request destination
    int httpCode = http.GET();//Send the request
    Serial.print("httpCode ");
    Serial.println(httpCode);
    if(httpCode == HTTP_CODE_OK){

      int len = http.getSize();// get lenght of document (is -1 when Server sends no Content-Length header)
      Serial.println("Size: " );
      Serial.println(len);
      // create buffer for read
      char jsonBuffer[256]; // verify this
      // get tcp stream
      WiFiClient * stream = http.getStreamPtr();
      // read all data from server
      while(http.connected() && (len > 0 || len == -1)) {
        // get available data size
        size_t size = stream->available();

        if(size) {
          // read up to 256 byte
          int c = stream->readBytes(jsonBuffer, ((size > sizeof(jsonBuffer)) ? sizeof(jsonBuffer) : size));
          // write it to Serial
          Serial.println(jsonBuffer);

          if(len > 0) {
            len -= c;
          }
        }
      }
    }*/
    if(checkAndUpdate("0.19.08")== HTTP_UPDATE_OK){
      Serial.println("Hoooray! I got new version! Restarting myself");
      ESP.restart();
    }
  }
}

void loop() {
  // wait for WiFi connection
  //if (WiFi.status() == WL_CONNECTED && !ok) {

    /*HTTPClient http;

    USE_SERIAL.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
    http.begin("https://api.github.com/repos/aavn/AirSniffer/releases/latest",gitHubFinger); //HTTP

    USE_SERIAL.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        USE_SERIAL.println(payload);
      }
    } else {
      USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    */
    //ok = (checkForUpdate("0.19.08") == HTTP_UPDATE_OK);
  //}

  delay(10000);
}
