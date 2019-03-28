#include "SnifferOTA.h"
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

HTTPUpdateResult checkAndUpdate(String currentVersion){
  HTTPUpdateResult result = HTTP_UPDATE_NO_UPDATES;
  WiFiClientSecure client;
  Serial.println("\nAttempt to check for update from the remote server");
  Serial.printf("Using fingerprint '%s'\n", GITHUB_FINGER);
  client.setFingerprint(GITHUB_FINGER);
  if ( !client.connect(SERVER_ADDRESS, 443) ) {
    Serial.println("connection failed");
    return HTTP_UPDATE_FAILED;
  }
  Serial.println("\n\n---------------------------------------------------------------------\n");
  Serial.println("REQUEST: \n" );
  String requestStr = String("GET ") ;
  requestStr.concat( RELEASE_URL);
  requestStr.concat(  " HTTP/1.1\r\n" );
  requestStr.concat( "Host: " );
  requestStr.concat( SERVER_ADDRESS );
  requestStr.concat( "\r\n");
  requestStr.concat("Accept: application/vnd.github.v3+json\r\n");
  requestStr.concat( "User-Agent: Sniffer\r\n");
  //requestStr.concat( "Connection: close\r\n");
  requestStr.concat( "\r\n");
  Serial.println("REQUEST: \n" );
  Serial.println(requestStr);
  Serial.println("\nAttempt to make a request to the remote server");
  
  client.print(requestStr);
  
  // If there are incoming bytes, print them
  Serial.println("\n\n---------------------------------------------------------------------\n");
  Serial.println("RESPONSE: \n" );
  // Read all the lines of the reply from server and print them to Serial
  int waitcount = 0;
  while (!client.available() && waitcount++ < 300) {
      Serial.println(".");
       delay(10);
  }
  String response = "";
  int count = 0;
  char c;
  bool dataFound = false;
  while ( client.available() ) {
     c = client.read();
    if(c == '{' && !dataFound){
      dataFound = true;
      response ="";
      Serial.println("Data found");
    }
    if(dataFound){
      response.concat(c);
    }
    //if(count == 0 && (response.endsWith("\r\n")||response.endsWith("\r\r")||response.endsWith("\n\n"))){
    //  Serial.println("Clearing String");
    //  response ="";
    //  count++;
    
    Serial.print(c);
    
  }
  
  Serial.println("\n\n---------------------------------------------------------------------\n");
  response.trim();
  Serial.println(response);
  Serial.println(response.length());
  //get latest release number
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(response.c_str());

  if (!root.success()) {
    Serial.println("parseObject() failed");
    result = HTTP_UPDATE_FAILED;
  }else{
    const char* latestVer = root[RELEASE_VERSION_KEY];
  
  
  
    
    //current version smaller than latest version
    if(strcmp(currentVersion.c_str(),latestVer)< 0){
      Serial.println("I am running on old version. I will call OTA to get latest version");
      Serial.print("Getting version: ");
      Serial.println(latestVer);
      
      //t_httpUpdate_return ret = ESPhttpUpdate.update(DOWNLOAD_URL,"",GITHUB_FINGER);
      t_httpUpdate_return ret = ESPhttpUpdate.update(client,SERVER_ADDRESS,443 ,DOWNLOAD_URL);
      Serial.println(F("\n---- ota update done ----\n"));
      Serial.println("==========");
      Serial.println(ret);
      Serial.println("==========");
      switch(ret) 
      {
        case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;
        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("Nothing to update");
          Serial.println(F("HTTP_UPDATE_NO_UPDATES"));
          break;
        case HTTP_UPDATE_OK:
          Serial.println("Update success!");
          Serial.println(F("HTTP_UPDATE_OK"));
          break;
        default:
            Serial.printf("Unexpected response code %d from ESPhttpUpdate.update\n",(int)ret);
            break;
    
      }
      result = ret;
    }else{
      Serial.print("I am running on latest version: " );
      Serial.print(currentVersion);
      Serial.println(". No need to update");
      result = HTTP_UPDATE_NO_UPDATES;
    }
  }
  
  return result;
}
