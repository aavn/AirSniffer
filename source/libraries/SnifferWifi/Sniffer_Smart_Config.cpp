#include "Sniffer_Smart_Config.h"
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
//Sniffer state
#define CONFIG_MODE 1
#define NORM_MODE 0
#define ONBOARDED -1

#define EEPROM_SIZE 1024


//private data
String configPage;

String macAddressJson;
String listWifiJson;
ESP8266WebServer server(httpPort);
int snifferState;
HubConfig * _oldConfig;
bool wifiConnected = false;

//private functions to expose APIs
void handleRoot();
void handlePwd();
void handleMAC();
void handleInfo();
void wifiConfigVerify();
void wifiConfigResult();
void scanWifi();
//server function
void launchWeb(void);

//void clearStoredWifi(void);

///////////////////

void setupAP(HubConfig* oldConfig) {
  Serial.println("Setting up SmartConfig");
  _oldConfig = oldConfig;
  //configPage.reserve(3000);
  configPage="";
  macAddressJson="{\"macAddress\":\"";
  macAddressJson.concat(_oldConfig->macStr);
  macAddressJson.concat("\"}");
  
  
  listWifiJson="[";
  WiFi.mode(WIFI_STA);
  if (WiFi.status() == WL_CONNECTED) {
	WiFi.disconnect();
  }
  delay(100);
  
  //jsonConfig="{'data':[";
  
  configPage = "";
  configPage.concat("<html lang='en-us'><head><title>Sniffer configuration</title>");
  configPage.concat("<style>h1 {color: CornflowerBlue ;} h2 {color: red;} .city {float: left;margin: 5px;padding: 15px;} ");
  configPage.concat(".inline {display: inline;} ");
  configPage.concat(".link-button {background: none; border: none; color: blue; text-decoration: underline; cursor: pointer; font-size: 1em; font-family: serif;} ");
  configPage.concat(".link-button:focus {outline: none;} ");
  configPage.concat(".link-button:active { color:red;} ");
  configPage.concat("</style></head><body><h1>Sniffer Configuration</h1>");
  configPage.concat("<div class='city'><h2>Choose a WIFI network for your Sniffer</h2><ol>");
  Serial.println("Prepare to scane network");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0){
    Serial.println("no networks found");
    configPage.concat("</ol>no networks found</div></body></html>");
  }else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
     {
		
      // Print SSID and RSSI for each network found - remove duplicated
      // It's complicated to use SET or Vector in Arduino C++ then here a work around
      String node = "name='ssid' value='";
      node.concat(WiFi.SSID(i));
      node.concat("'>");
	  
      if (configPage.indexOf(node) <=0) {
		  if (i> 0 && i<n-1){
			listWifiJson.concat(',');
		  }
		  
		  Serial.print(i + 1);
		  Serial.print(": ");
		  Serial.print(WiFi.SSID(i));
		  Serial.print(" (");
		  Serial.print(WiFi.RSSI(i));
		  Serial.print(")");
		  
		  Serial.println(WiFi.encryptionType(i));
		  //Serial.print("Channel: ");
		  //Serial.println(WiFi.channel(i));
		  //Serial.print("BSSIDstr: ");
		  //Serial.println(WiFi.BSSIDstr(i));
		  delay(10);
		  //configPage.concat("<li><a href='/inputPwd/"+WiFi.SSID(i)+"'>"+WiFi.SSID(i)+"</a></li>");
		  configPage.concat("<li><form method='post' action='/inputPwd' class='inline'>");
		  configPage.concat("<input type='hidden' name='ssid' value='"+ WiFi.SSID(i)+ "'>");
		  configPage.concat("<button type='submit' name='submit_param' value='submit_value' class='link-button'>");
		  configPage.concat(WiFi.SSID(i)+"</button></form></li>");
		  
		  listWifiJson.concat("{\"ssid\":\"" + WiFi.SSID(i) + "\"}");
		  
	  }
		
     }
     configPage.concat("</ol></div></body></html>");
  }
  listWifiJson.concat("]");
    
  delay(100);
  WiFi.softAP(smartConfigSSID);
  Serial.println("softap");
  Serial.println("");
  launchWeb();
  Serial.println("over");
}


void launchWeb(void) {
    Serial.println("");
    Serial.println("Webserver started!");
    Serial.println(WiFi.softAPIP());

    // Start the server
    server.on("/", handleRoot);
    server.on("/inputPwd",handlePwd);
    server.on("/wifi_config/verify",wifiConfigVerify);
    server.on("/wifi_config/result",wifiConfigResult);
    server.on("/api/sniffer/mac",handleMAC);
	server.on("/api/sniffer/listWifi",scanWifi);
	server.on("/api/sniffer/info",handleInfo);
        
    server.begin();
    Serial.println("Server started");   

}
void handleInfo(void){
	String infoJson = "{\"code\":";
	if(strlen(_oldConfig->code)>0){
		infoJson.concat("\"");
		infoJson.concat(_oldConfig->code );
		infoJson.concat("\",");
	}else{
		
		infoJson.concat("null,");
	}
	infoJson.concat("\"gpsLocation\": {\"latitude\":" );
	if(strlen(_oldConfig->latitude)>0){
		infoJson.concat(_oldConfig->latitude );
		infoJson.concat(",");
	}else{
		infoJson.concat("null,");
	}
	infoJson.concat("\"longitude\": ");
	if(strlen(_oldConfig->longitude)>0){
		infoJson.concat(_oldConfig->longitude );
		infoJson.concat("},");
	}else{
		infoJson.concat("null},");
	}
	
	infoJson.concat("\"macAddress\":");
	if(strlen(_oldConfig->macStr)>0){
		infoJson.concat("\"");
		infoJson.concat(_oldConfig->macStr );
		infoJson.concat("\",");
	}else{
		
		infoJson.concat("null,");
	}
	infoJson.concat("\"ota\": ");
	infoJson.concat(_oldConfig->ota );
	infoJson.concat("}");
	Serial.println(infoJson);
	server.send(200, "text/json",infoJson );
}
void handleMAC(void){
	
	
	server.send(200, "text/json",macAddressJson );
}
void scanWifi(void){
	server.send(200, "text/json", listWifiJson);
}
void wifiConfigVerify(){
	HubConfig smartConfig;

	String verifyPage = "";
  	verifyPage.concat("<html lang='en-us'><head><title>Sniffer configuration</title>");
  	verifyPage.concat("<meta http-equiv='refresh' content='30;url=/wifi_config/result'>");
  	verifyPage.concat("</head><body><h1>Sniffer is verifying Wifi Configuration. Please be patient! Result is coming ...</h1>");
  	verifyPage.concat("</body></html>");
  	
  	
  	String ssid, pwd, code, latitude, longitude, ota;
    Serial.println("Config Args: ");
    for(int i = 0; i < server.args(); i++){
      Serial.print(server.argName(i));
      Serial.print(": ");
      Serial.println(server.arg(i));
    }
    Serial.println();
    Serial.println("Config Headers: ");
    for(int i = 0; i < server.headers(); i++){
      Serial.print(server.headerName(i));
      Serial.print(": ");
      Serial.println(server.header(i));
    }
    Serial.println();
    ssid = server.arg("ssid");
    pwd = server.arg("pwd");
	code = server.arg("code");
    latitude = server.arg("lat");
	longitude = server.arg("long");
	
	if(server.hasArg("ota")){
		Serial.println("Having OTA");
		smartConfig.ota = server.arg("ota").toInt();
	}else{
		smartConfig.ota = 0;
	}
    
    strcpy(smartConfig.ssid,ssid.c_str());
	strcpy(smartConfig.pwd,pwd.c_str());
	//send response to client
	server.send(200, "text/html", verifyPage);
	
	WiFi.begin(smartConfig.ssid,smartConfig.pwd);
	int c = 0;
	Serial.print("Wifi connecting");
	while ( c < 30 ) {
    	if (WiFi.status() == WL_CONNECTED) { 
    		wifiConnected = true;
      		break; 
    	} 
    	Serial.print(".");
    	delay(500);
    	c++;
  	}
  	if (wifiConnected) {
  		Serial.println("Wifi connected");
  		
		strcpy(smartConfig.code,code.c_str());
	    strcpy(smartConfig.latitude,latitude.c_str());
		strcpy(smartConfig.longitude,longitude.c_str());		
	    strcpy(smartConfig.macStr,_oldConfig->macStr);

	    smartConfig.mode = NORM_MODE;
	    storeConfig(&smartConfig);
  	}
}

void wifiConfigResult(){
	String htmlErr = "";
  	htmlErr.concat("<html lang='en-us'><head><title>Sniffer configuration</title>");
  	htmlErr.concat("</head><body>");
  	htmlErr.concat("<h1>Configuration result:</h1>");
    htmlErr.concat("<h2>Wrong configuration! Please go back try again!</h2>");
    htmlErr.concat("<input type='button' onclick='location.href='/';' value='Try again' />");
  	htmlErr.concat("</body></html>");

	String htmlStr = "";
  	htmlStr.concat("<html lang='en-us'><head><title>Sniffer configuration</title>");
  	htmlStr.concat("</head><body>");
  	htmlStr.concat("<h1>Configuration result:</h1>");
    htmlStr.concat("<h2>Wifi connected. Configuration stored. Restarting!</h2>");
  	htmlStr.concat("</body></html>");

	if (wifiConnected) {
		snifferState = ONBOARDED;
		Serial.println("Wifi connected");
	    server.send(200, "text/html", htmlStr);
	    delay(500);
		
		WiFi.mode(WIFI_STA);
		
	} else {
  		Serial.println("Wifi NOT connected");
		server.send(200, "text/html", htmlErr);
	}
}

void handlePwd(){
 
  Serial.println("Args: ");
  for(int i = 0; i < server.args(); i++){
    Serial.print(server.argName(i));
    Serial.print(": ");
    Serial.println(server.arg(i));
  }
  Serial.println();
  String decodedStr = server.arg("ssid");
  
  
  String htmlPage ="<html><body><h2>Enter Sniffer Config</h2>";

	htmlPage.concat("<form method='POST' action='/wifi_config/verify'>");
	htmlPage.concat("<table><tr><td>Wifi</td><td colspan=2><input readonly maxlength='36' name='ssid' id='ssid' value='" + decodedStr +"'/></td></tr>");
	htmlPage.concat("<tr><td>Password</td><td colspan=2><input maxlength='50' type='text' name='pwd' id='pwd' /></td></tr>");
	htmlPage.concat("<tr><td colspan=3><hr></td></tr>");
	decodedStr="";
	decodedStr.concat (_oldConfig->code);
	htmlPage.concat("<tr><td>Code</td><td colspan=2><input maxlength='16' type='text' name='code' id='code' value='" + decodedStr +"'/></td></tr>");
	htmlPage.concat("<tr><td rowspan=2><b>GPS</b></td>");
	decodedStr="";
	decodedStr.concat (_oldConfig->latitude);
	htmlPage.concat("<td>Latitude</td><td><input maxlength='11' type='text' name='lat' id='lat' value='" + decodedStr +"'/></td></tr>");
	decodedStr="";
	decodedStr.concat (_oldConfig->longitude);
	htmlPage.concat("<tr><td>Longitude</td><td><input maxlength='11' type='text' name='long' id='long' value='" + decodedStr +"'/></td></tr>");
	decodedStr="";
	if(_oldConfig->ota == 1){
		decodedStr.concat("checked") ;
	}
	
	htmlPage.concat("<tr><td>Auto update firmware?</td><td><input type='checkbox' name='ota' id='ota' value='1' " + decodedStr +" /></td>");
	htmlPage.concat("<td style='color:crimson;word-wrap: break-word;'>If choosen, Sniffer will automatically download  firmware from server whenever new version released</td></tr>");
	htmlPage.concat("<tr><td></td><td><input type='submit' value='Connect'/></td></tr>");
	htmlPage.concat("</table></form></body></html>");
  
  
  server.send(200, "text/html",htmlPage);
}
void handleRoot() {
  //server.send(200, "text/html", "<h1>You are connected</h1>");
  //Serial.println("Sending: ");
  //Serial.println(configPage);
  server.send(200, "text/html",configPage);
}
/*void handleConfig(){
    server.send(200, "text/plain", jsonConfig);
}*/

void loadConfig(HubConfig* smartConfig){
	EEPROM.get(0,*smartConfig);
	if(strlen(smartConfig->macStr) == 0){
		stpcpy(smartConfig->macStr,WiFi.macAddress().c_str());
	}
}
void storeConfig(HubConfig* smartConfig){
	EEPROM.put(0,*smartConfig);
	EEPROM.commit();
	delay(100);
}

/*void clearStoredWifi(){
	HubConfig * dummy;
  Serial.println("\nclearing wifi config only");
  int i;
        for ( i = 0; i < sizeof(dummy->mode)+sizeof(dummy->ssid)+sizeof(dummy->pwd); ++i) { 
        	Serial.print(".");
			EEPROM.write(i, 0); 
		}
	
  EEPROM.commit();
  delay(100);
  Serial.print("Clear Wifi size: ");
  Serial.println(i);
}*/
void clearStoredEEPROM(){
	//HubConfig * dummy;
  Serial.println("\nclearing all config data");
        for (int i = 0; i < EEPROM_SIZE; ++i) { 
			EEPROM.write(i, 0); 
		}

  EEPROM.commit();
  delay(100);
}

void handleSmartConfigClient(){
	server.handleClient();
}
void initSnifferConfig(HubConfig* smartConfig){
  Serial.println("initSnifferConfig ");
  EEPROM.begin(EEPROM_SIZE);
  delay(10);
  loadConfig(smartConfig);
  snifferState = smartConfig->mode;
  delay(50);
  
}
void prepareSmartConfig(HubConfig* smartConfig){
	Serial.println("prepareSmartConfig ");
	smartConfig->mode = CONFIG_MODE;
	smartConfig->ssid[0] = '\0';
	smartConfig->pwd[0] = '\0';
	EEPROM.write(0, smartConfig->mode);
	EEPROM.commit();
	printConfig(smartConfig);
    
}
void printConfig(HubConfig* smartConfig){
	Serial.print("MODE: "); Serial.println(smartConfig->mode,DEC);
	Serial.print("OTA: "); Serial.println(smartConfig->ota,DEC);
	Serial.print("SSID: "); Serial.println(smartConfig->ssid);
	Serial.print("PWD: "); Serial.println(smartConfig->pwd);
	Serial.print("CODE: "); Serial.println(smartConfig->code);
	Serial.print("LAT: "); Serial.println(smartConfig->latitude);
	Serial.print("LONG: "); Serial.println(smartConfig->longitude);
	Serial.print("MAC: "); Serial.println(smartConfig->macStr);
}
bool needRestart(){
	return snifferState == ONBOARDED;
}
