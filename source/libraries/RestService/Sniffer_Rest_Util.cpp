#include "Sniffer_Rest_Util.h"
#include "Sniffer_Rest_Util_staging.h"
#include <SPI.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include "Sniffer_Data_Util.h"
#include "Sniffer_Rest_Property.h"
#include "Sniffer_Rest_Property_staging.h"

#define STATUS_201 "HTTP/1.1 201 Created"
bool saveData(Environment * envirData, RestProperty * restProperty)
{
  // Attempt to make a connection to the remote server
  Serial.println("\nAttempt to make a connection to the remote server");
	WiFiClientSecure client;
	client.setFingerprint(finger);
  if ( !client.connect(serverAddress, serverPort) ) {
    Serial.println("connection failed");
	return false;
  }
  Serial.println("\n\n---------------------------------------------------------------------\n");
  Serial.println("REQUEST: \n" );
  // We now create a URI for the request
  //String postData = "{\"source\": {\"senderCode\": \"143253\",\"netAddress\": \"192.168.1.1\"},\"data\": [{\"symbolCode\": \"O3\",\"value\": 4.1}, {\"symbolCode\": \"PM2.5\",\"value\": 55}]}";
  char data[600];
  char strLenght[5];
  formatAAVNData(data, envirData,restProperty);
  Serial.print("Data: ");
  Serial.println(data);
  sprintf (strLenght,"%u\0",strlen(data));
  //String len = String(strlen(data));
  String requestStr = String("POST ");
	requestStr.concat(snifferUrl);
	requestStr.concat(" HTTP/1.1\r\nHost: ");
	requestStr.concat(serverAddress);
	requestStr.concat( "\r\nUser-Agent: SnifferHub\r\nContent-Length: " );
	requestStr.concat(strLenght);
	requestStr.concat("\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n");
	requestStr.concat(data);
    requestStr.concat("\r\n");
  Serial.println(requestStr);
  // This will send the request to the server
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
  while ( client.available() ) {
    char c = client.read();
	response.concat(c);
    //Serial.print(c);
    count++;
  }
  Serial.println(response);
  client.stop();
  response.trim();
  if(response.startsWith(STATUS_201)){
	  return true;
  }else{
	  return false;
  }
}
bool saveData_staging(Environment * envirData, RestProperty * restProperty)
{
  // Attempt to make a connection to the remote server
  Serial.println("\nAttempt to make a connection to the staging server");
  WiFiClient client; 
  if ( !client.connect(serverAddress_staging, serverPort_staging) ) {
    Serial.println("connection failed");
	return false;
  }
  Serial.println("\n\n---------------------------------------------------------------------\n");
  Serial.println("REQUEST: \n" );
  // We now create a URI for the request
  //String postData = "{\"source\": {\"senderCode\": \"143253\",\"netAddress\": \"192.168.1.1\"},\"data\": [{\"symbolCode\": \"O3\",\"value\": 4.1}, {\"symbolCode\": \"PM2.5\",\"value\": 55}]}";
  char data[600];
  char strLenght[5];
  formatAAVNData(data, envirData,restProperty);
  Serial.print("Data: ");
  Serial.println(data);
  sprintf (strLenght,"%u\0",strlen(data));
  //String len = String(strlen(data));
  String requestStr = String("POST ");
	requestStr.concat(snifferUrl_staging);
	requestStr.concat(" HTTP/1.1\r\nHost: ");
	requestStr.concat(serverAddress_staging);
	requestStr.concat( "\r\nUser-Agent: SnifferHub\r\nContent-Length: " );
	requestStr.concat(strLenght);
	requestStr.concat("\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n");
	requestStr.concat(data);
    requestStr.concat("\r\n");
  Serial.println(requestStr);
  // This will send the request to the server
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
  while ( client.available() ) {
    char c = client.read();
	response.concat(c);
    //Serial.print(c);
    count++;
  }
  Serial.println(response);
  client.stop();
  response.trim();
  if(response.startsWith(STATUS_201)){
	  return true;
  }else{
	  return false;
  }
}
