#include "Sniffer_Smart_Config.h"
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
//Sniffer state
#define CONFIG_MODE 1
#define NORM_MODE 0
#define ONBOARDED -1

//private data
String configPage;
String jsonConfig;
String macAddressJson;
ESP8266WebServer server(httpPort);
int snifferState;
HubConfig * _oldConfig;
bool wifiConnected = false;

//private functions
void handleRoot();
void handlePwd();
void handleConfig();
void wifiConfigVerify();
void wifiConfigResult();
void launchWeb(void);
//void clearStoredConfig(void);
//void clearStoredWifi(void);

///////////////////

void setupAP(HubConfig* oldConfig) {
  _oldConfig = oldConfig;
  configPage.reserve(3000);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  jsonConfig="{\"data\":[";
  
  configPage = "";
  configPage.concat("<html lang=\"en-us\"><head><title>Sniffer configuration</title>");
  configPage.concat("<style>h1 {color: CornflowerBlue ;} h2 {color: red;} .city {float: left;margin: 5px;padding: 15px;} ");
  configPage.concat(".inline {display: inline;} ");
  configPage.concat(".link-button {background: none; border: none; color: blue; text-decoration: underline; cursor: pointer; font-size: 1em; font-family: serif;} ");
  configPage.concat(".link-button:focus {outline: none;} ");
  configPage.concat(".link-button:active { color:red;} ");
  configPage.concat("</style></head><body><h1>Sniffer Configuration</h1>");
  configPage.concat("<div class=\"city\"><h2>Choose a WIFI network for your Sniffer</h2><ol>");
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
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
      //configPage.concat("<li><a href=\"/inputPwd/"+WiFi.SSID(i)+"\">"+WiFi.SSID(i)+"</a></li>");
      configPage.concat("<li><form method=\"post\" action=\"/inputPwd\" class=\"inline\">");
      configPage.concat("<input type=\"hidden\" name=\"ssid\" value=\""+ WiFi.SSID(i)+ "\">");
      configPage.concat("<button type=\"submit\" name=\"submit_param\" value=\"submit_value\" class=\"link-button\">");
      configPage.concat(WiFi.SSID(i)+"</button></form></li>");
      
      if (i==n-1)
		jsonConfig.concat("\"" + WiFi.SSID(i) + "\"");
	  else
		jsonConfig.concat("\"" + WiFi.SSID(i) + "\",");
     }
     configPage.concat("</ol></div></body></html>");
  }
  
  jsonConfig.concat("],");
  
  jsonConfig.concat("\"mac_address\":");
  jsonConfig.concat("\"" + WiFi.macAddress() + "\"");
  jsonConfig.concat("}");
    
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
    server.on("/config",handleConfig);
        
    server.begin();
    Serial.println("Server started");   

}

void wifiConfigVerify(){
	HubConfig smartConfig;

	String verifyPage = "";
  	verifyPage.concat("<html lang=\"en-us\"><head><title>Sniffer configuration</title>");
  	verifyPage.concat("<meta http-equiv=\"refresh\" content=\"15;url=/wifi_config/result\">");
  	verifyPage.concat("</head><body><h1>Sniffer is verifying Wifi Configuration. Please be patient! Result is coming ...</h1>");
  	verifyPage.concat("</body></html>");
  	
  	
  	String ssid, pwd, code, latitude, longitude, macStr;
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
    macStr = server.arg("mac");
    
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
	    strcpy(smartConfig.macStr,macStr.c_str());

	    smartConfig.mode = NORM_MODE;
	    storeConfig(&smartConfig);
  	}
}

void wifiConfigResult(){
	String htmlErr = "";
  	htmlErr.concat("<html lang=\"en-us\"><head><title>Sniffer configuration</title>");
  	htmlErr.concat("</head><body>");
  	htmlErr.concat("<h1>Configuration result:</h1>");
    htmlErr.concat("<h2>Wrong configuration! Please go back try again!</h2>");
    htmlErr.concat("<input type=\"button\" onclick=\"location.href='/';\" value=\"Try again\" />");
  	htmlErr.concat("</body></html>");

	String htmlStr = "";
  	htmlStr.concat("<html lang=\"en-us\"><head><title>Sniffer configuration</title>");
  	htmlStr.concat("</head><body>");
  	htmlStr.concat("<h1>Configuration result:</h1>");
    htmlStr.concat("<h2>Wifi connected. Configuration stored. Restarting!</h2>");
  	htmlStr.concat("</body></html>");

	if (wifiConnected) {
		Serial.println("Wifi connected");
	    server.send(200, "text/html", htmlStr);
	    delay(500);
		snifferState = ONBOARDED;
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

	htmlPage.concat("<form method=\"POST\" action=\"/wifi_config/verify\">");
	htmlPage.concat("<table><tr><td>Wifi</td><td colspan=2><input readonly name=\"ssid\" id=\"ssid\" value=\"" + decodedStr +"\"\\></td></tr>");
	htmlPage.concat("<tr><td>Password</td><td colspan=2><input type=\"text\" name=\"pwd\" id=\"pwd\" \"\\></td></tr>");
	htmlPage.concat("<tr><td colspan=3><hr></td></tr>");
	decodedStr="";
	decodedStr.concat (_oldConfig->code);
	htmlPage.concat("<tr><td>Code</td><td colspan=2><input type=\"text\" name=\"code\" id=\"code\" value=\"" + decodedStr +"\"\\></td></tr>");
	htmlPage.concat("<tr><td rowspan=2><b>GPS</b></td>");
	decodedStr="";
	decodedStr.concat (_oldConfig->latitude);
	htmlPage.concat("<td>Latitude</td><td><input type=\"text\" name=\"lat\" id=\"lat\" value=\"" + decodedStr +"\"\\></td></tr>");
	decodedStr="";
	decodedStr.concat (_oldConfig->longitude);
	htmlPage.concat("<tr><td>Longitude</td><td><input type=\"text\" name=\"long\" id=\"long\" value=\"" + decodedStr +"\"\\></td></tr>");
	decodedStr="";
	decodedStr.concat (_oldConfig->macStr);
	htmlPage.concat("<tr><td>MAC</td><td colspan=2><input type=\"text\" name=\"mac\" id=\"mac\" value=\"" + decodedStr +"\"\\></td></tr>");
	htmlPage.concat("<tr><td></td><td><input type=\"submit\" value=\"Connect\"\\></td></tr>");
	htmlPage.concat("</table></form></body></html>");
  
  
  server.send(200, "text/html",htmlPage);
}
/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
  //server.send(200, "text/html", "<h1>You are connected</h1>");
  //Serial.println("Sending: ");
  //Serial.println(configPage);
  server.send(200, "text/html",configPage);
}
void handleConfig(){
    server.send(200, "text/plain", jsonConfig);
}

void loadConfig(HubConfig* smartConfig){
  //read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");
  smartConfig->mode = (uint8_t)(EEPROM.read(0));
  Serial.print("Mode: ");
  Serial.println(smartConfig->mode);
  Serial.print("Reading: ");
  Serial.println(EEPROM.read(0));
  int usedByteCount = sizeof(smartConfig->mode);

  String esid="",epass = "",ecode ="",elatitude="", elongitude="", emac="";
  for (int i = usedByteCount; i < usedByteCount + sizeof(smartConfig->ssid); ++i)
    {
      if(EEPROM.read(i) != 0){
        esid += char(EEPROM.read(i));
      }
    }
  esid.trim();
  Serial.print("eSSID: ");
  Serial.print(esid);Serial.println("\\END");  
  Serial.print("esid length: ");
  Serial.println(esid.length());
  Serial.println("Reading EEPROM pass");
  //read Password
  usedByteCount += sizeof(smartConfig->ssid);
  for (int i = usedByteCount; i <usedByteCount + sizeof(smartConfig->pwd); ++i)
  {
	  if(EEPROM.read(i) != 0){
        epass += char(EEPROM.read(i));
      }
//       epass += char(EEPROM.read(i));
  }
  Serial.print("ePASS: ");
  Serial.print(epass);  
  Serial.println("\\END");  
  Serial.print("epass length: ");
  Serial.println(epass.length());
  epass.trim();
  //read Sniffer Code
  usedByteCount += sizeof(smartConfig->pwd);
  for (int i = usedByteCount; i <usedByteCount + sizeof(smartConfig->code); ++i)
    {
//	  if(EEPROM.read(i) != 0){
//        ecode += char(EEPROM.read(i));
 //     }
    
 		if(isAscii(char(EEPROM.read(i))))
 			ecode += char(EEPROM.read(i));
    }
  Serial.print("eCODE: ");
  Serial.print(ecode);  
  Serial.println("\\END");  
  Serial.print("ecode length: ");
  Serial.println(ecode.length());
  ecode.trim();
  //read Sniffer Latitude
  usedByteCount += sizeof(smartConfig->code);
  for (int i = usedByteCount; i <usedByteCount + sizeof(smartConfig->latitude); ++i)
    {
	 // if(EEPROM.read(i) != 0){
    //    elatitude += char(EEPROM.read(i));
    //  }
 		if(isAscii(char(EEPROM.read(i))))
 			elatitude += char(EEPROM.read(i));
    }
  Serial.print("eLAT: ");
  Serial.print(elatitude);  
  Serial.println("\\END"); 
  Serial.print("elatitude length: ");
  Serial.println(elatitude.length());  
  elatitude.trim();
  //read Sniffer Longitude
  usedByteCount += sizeof(smartConfig->latitude);
  for (int i = usedByteCount; i <usedByteCount + sizeof(smartConfig->longitude); ++i)
    {
	 // if(EEPROM.read(i) != 0){
      //  elongitude += char(EEPROM.read(i));
    //  }
    
 		if(isAscii(char(EEPROM.read(i))))
 			elongitude += char(EEPROM.read(i));
    }
  Serial.print("eLONG: ");
  Serial.print(elongitude);  
  Serial.println("\\END");  
  Serial.print("elongitude length: ");
  Serial.println(elongitude.length()); 
  elongitude.trim();
  //read Sniffer MAC
  usedByteCount += sizeof(smartConfig->longitude);
  for (int i = usedByteCount; i <usedByteCount + sizeof(smartConfig->macStr); ++i)
    {
	 // if(EEPROM.read(i) != 0){
     //   emac += char(EEPROM.read(i));
     // }
    
 		if(isAscii(char(EEPROM.read(i))))
 			emac += char(EEPROM.read(i));
    }
  Serial.print("eMAC: ");
  Serial.print(emac);  
  Serial.println("\\END"); 
  Serial.print("emac length: ");
  Serial.println(emac.length());   
  emac.trim();
  
  strcpy(smartConfig->ssid,esid.c_str());
  strcpy(smartConfig->pwd,epass.c_str());
  strcpy(smartConfig->code,ecode.c_str());
  strcpy(smartConfig->latitude,elatitude.c_str());
  strcpy(smartConfig->longitude,elongitude.c_str());
  strcpy(smartConfig->macStr,emac.c_str());

  printConfig(smartConfig);
}
void storeConfig(HubConfig* smartConfig){
  printConfig(smartConfig);
  Serial.println("clearing eeprom");
  
  String qsid = "",qpass="", qcode ="",qlatitude="", qlongitude="", qmac="";
  
  int size;
  for (size = 0; size < sizeof(smartConfig->mode)+sizeof(smartConfig->ssid)
					+sizeof(smartConfig->pwd)+sizeof(smartConfig->code)+sizeof(smartConfig->latitude)
					+sizeof(smartConfig->longitude)+sizeof(smartConfig->macStr); 
			++size) { 
  	EEPROM.write(size, '\0'); 
  	Serial.print(".");
  }
  
  EEPROM.commit();
  delay(100);
  Serial.print("Clear EEPROM size: ");
  Serial.println(size);
  
  EEPROM.write(0, smartConfig->mode);
  int usedByteCount = sizeof(smartConfig->mode);
  qsid.concat(smartConfig->ssid);
  Serial.println(qsid);
  Serial.print("");
  
  qpass.concat(smartConfig->pwd);
  Serial.print(qpass);
  Serial.println("");
  qcode.concat(smartConfig->code);
  Serial.print(qcode);
  Serial.println("");
  qlatitude.concat(smartConfig->latitude);
  Serial.print(qlatitude);
  Serial.println("");
  qlongitude.concat(smartConfig->longitude);
  Serial.print(qlongitude);
  Serial.println("");
  qmac.concat(smartConfig->macStr);
  Serial.print(qmac);
  Serial.println("");
  
  Serial.print("writing eeprom ssid from byte :");
  Serial.println(usedByteCount);
  
  int i;
  for ( i = 0; i < qsid.length(); ++i)
    {
      EEPROM.write(usedByteCount+i, qsid[i]);
//       Serial.print("Wrote: ");
//       Serial.println(qsid[i]); 
    }
  EEPROM.write(usedByteCount+i, '\0');
  usedByteCount += sizeof(smartConfig->ssid);
  
  Serial.print("writing eeprom pass from byte :");
  Serial.println(usedByteCount);
  for ( i = 0; i < qpass.length(); ++i)
    {
      EEPROM.write(usedByteCount+i, qpass[i]);
//       Serial.print("Wrote: ");
//       Serial.println(qpass[i]); 
    }
   
  EEPROM.write(usedByteCount+i, '\0');   
  usedByteCount += sizeof(smartConfig->pwd);
  
  
  Serial.print("writing eeprom code from byte :");
  Serial.println(usedByteCount);
  for ( i = 0; i < qcode.length(); ++i)
    {
      EEPROM.write(usedByteCount+i, qcode[i]);
//       Serial.print("Wrote: ");
//       Serial.println(qcode[i]); 
    }

  EEPROM.write(usedByteCount+i, '\0');
  usedByteCount += sizeof(smartConfig->code);
  
  Serial.print("writing eeprom lat from byte :");
  Serial.println(usedByteCount);
  int j;
  for (j = 0; j < qlatitude.length(); ++j)
    {
      EEPROM.write(usedByteCount+j, qlatitude[j]);
//       Serial.print("Wrote: ");
//       Serial.println(qlatitude[j]); 
    }
	EEPROM.write(usedByteCount+j, '\0');
	
  usedByteCount += sizeof(smartConfig->latitude);
  Serial.print("writing eeprom long from byte :");
  Serial.println(usedByteCount);
  
  for (i = 0; i < qlongitude.length(); ++i)
    {
      EEPROM.write(usedByteCount+i, qlongitude[i]);
//       Serial.print("Wrote: ");
//       Serial.println(qlongitude[i]); 
    }
	EEPROM.write(usedByteCount+i, '\0');
  usedByteCount += sizeof(smartConfig->longitude);
  Serial.print("writing eeprom mac from byte :");
  Serial.println(usedByteCount);
  for ( i = 0; i < qmac.length(); ++i)
    {
      EEPROM.write(usedByteCount+i, qmac[i]);
//       Serial.print("Wrote: ");
//       Serial.println(qmac[i]); 
    }
  EEPROM.commit();
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
}
void clearStoredConfig(){
	HubConfig * dummy;
  Serial.println("\nclearing all config data");
        for (int i = 0; i < sizeof(HubConfig); ++i) { 
			EEPROM.write(i, 0); 
		}

  EEPROM.commit();
  //delay(100);
}*/
bool isConfigMode(HubConfig* smartConfig){
	return smartConfig->mode == CONFIG_MODE;
}
void handleSmartConfigClient(){
	server.handleClient();
}
void initSnifferConfig(HubConfig* smartConfig){
  Serial.println("initSnifferConfig ");
  EEPROM.begin(1024);
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
	//smartConfig->code[0] = '\0';
	//smartConfig->latitude[0] = '\0';
	//smartConfig->longitude[0] = '\0';
	//smartConfig->macStr[0] = '\0';
	//clearStoredConfig();
    //hubConfig.mode = NORM_MODE;
	//WiFi.mode(WIFI_OFF);
	//delay(50);
	//clearStoredWifi();
	printConfig(smartConfig);
    //storeConfig(smartConfig);
    
}
void printConfig(HubConfig* smartConfig){
	Serial.print("MODE: "); Serial.println(smartConfig->mode);
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