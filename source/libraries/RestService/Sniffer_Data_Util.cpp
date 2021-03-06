#include "Sniffer_Data_Util.h"
#include "Sniffer_Rest_Constant.h"
#include <ArduinoJson.h>
#include <Sniffer_Rest_Property.h>
#include <TimeLib.h>
#include <EEPROM.h>

void formatAAVNData(char * dataStr, Environment * envirData, RestProperty * restProperty){
  // put your setup code here, to run once:
  const size_t capacity = DATA_SIZE + 100;//JSON_ARRAY_SIZE(4) + 4*JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(2) + 5*JSON_OBJECT_SIZE(3);
  char dateTimeStr[25];
  if(year(envirData->time) == 1970 || envirData->time == 0){
	strcpy(dateTimeStr,"");
  }else{
  formatDate(envirData->time,dateTimeStr );
  }
  
  Serial.println("CAPACITY: ");
  Serial.println(capacity);
  DynamicJsonDocument doc(capacity);
  
  JsonObject source = doc.createNestedObject(SOURCE_KEY);
  source[SOURCE_ID_KEY] = restProperty->sender_pro;
  //source[MAC_KEY] = restProperty->mac_str_pro;
  
  JsonObject source_gpsLocation = source.createNestedObject(GPS_KEY);
  source_gpsLocation[LAT_KEY] = restProperty->latitude_pro;
  source_gpsLocation[LONG_KEY] = restProperty->longitude_pro;
  
  JsonArray values = doc.createNestedArray(VALS_KEY);
  if(envirData->humidity > HUM_MIN && envirData->humidity <= HUM_MAX){
    JsonObject humVal = values.createNestedObject();
    humVal[CODE_KEY] = HUM_KEY;
    humVal[SENSOR_KEY] = restProperty->TEMP_SENSOR_pro;
    
    JsonObject humValData = humVal.createNestedObject(VAL_KEY);
    humValData["value"] = envirData->humidity;
	if(strlen(dateTimeStr) > 0){
		humValData[MEASURE_KEY] = dateTimeStr;
	}
  }else{
    Serial.println("/*******************************/");
    Serial.print("ERROR reading HUM: ");
    Serial.println(envirData->humidity);
    Serial.println("/*******************************/");
  }
  if(envirData->temperature > TEMP_MIN && envirData->temperature <= TEMP_MAX){
    JsonObject tempVal = values.createNestedObject();
    tempVal[CODE_KEY] = TEMP_KEY;
    tempVal[SENSOR_KEY] = restProperty->TEMP_SENSOR_pro;
    
    JsonObject tempValData = tempVal.createNestedObject(VAL_KEY);
    tempValData["value"] = envirData->temperature;
	if(strlen(dateTimeStr) > 0){
		tempValData[MEASURE_KEY] = dateTimeStr;
	}
  }else{
    Serial.println("/*******************************/");
    Serial.print("ERROR reading TEMP: ");
    Serial.println(envirData->temperature);
    Serial.println("/*******************************/");
  }
  if(envirData->novaPm25 > PM_MIN && envirData->novaPm25 <= PM_MAX){
    JsonObject pm25Val = values.createNestedObject();
    pm25Val[CODE_KEY] = PM25_KEY;
    pm25Val[SENSOR_KEY] = restProperty->PM_SENSOR_pro;
    
    JsonObject pm25ValData = pm25Val.createNestedObject(VAL_KEY);
    pm25ValData["value"] = envirData->novaPm25;
	if(strlen(dateTimeStr) > 0){
		pm25ValData[MEASURE_KEY] = dateTimeStr;
	}
  }else{
    Serial.println("/*******************************/");
    Serial.print("ERROR reading PM2.5: ");
    Serial.println(envirData->novaPm25);
    Serial.println("/*******************************/");
  }
  if(envirData->novaPm10 > PM_MIN && envirData->novaPm10 <= PM_MAX){
    JsonObject pm10Val = values.createNestedObject();
    pm10Val[CODE_KEY] = PM10_KEY;
    pm10Val[SENSOR_KEY] = restProperty->PM_SENSOR_pro;
    
    JsonObject pm10ValData = pm10Val.createNestedObject(VAL_KEY);
    pm10ValData["value"] = envirData->novaPm10;
	if(strlen(dateTimeStr) > 0){
		pm10ValData[MEASURE_KEY] = dateTimeStr;
	}
  }else{
    Serial.println("/*******************************/");
    Serial.print("ERROR reading PM10: ");
    Serial.println(envirData->novaPm10);
    Serial.println("/*******************************/");
  }
  if(envirData->ozone > OZONE_MIN && envirData->ozone <= OZONE_MAX){
    JsonObject ozoneVal = values.createNestedObject();
    ozoneVal[CODE_KEY] = OZONE_KEY;
    ozoneVal[SENSOR_KEY] = restProperty->OZONE_SENSOR_pro;
    
    JsonObject ozoneData = ozoneVal.createNestedObject(VAL_KEY);
    ozoneData["value"] = envirData->ozone;
	if(strlen(dateTimeStr) > 0){
		ozoneData[MEASURE_KEY] = dateTimeStr;
	}
  }else{
    Serial.println("/*******************************/");
    Serial.print("ERROR reading Ozone: ");
    Serial.println(envirData->ozone);
    Serial.println("/*******************************/");
  }
  //serializeJson(doc, Serial);
  
  serializeJson(doc, dataStr,DATA_SIZE);
}

void printData(Environment * envirData){
  //int index=0;
  //index=index+1;
  //Serial.print(index );
  char dateTimeStr[25];
  Serial.print(" Nova_PM2.5: ");
  Serial.print(envirData->novaPm25);
  Serial.print(" Nova_PM10: ");
  Serial.print(envirData->novaPm10);
  Serial.print(" Temperature: ");
  Serial.print(envirData->temperature);
  Serial.print(" Humidity: ");
  Serial.print(envirData->humidity);
  Serial.print(" Ozone: ");
  Serial.print(envirData->ozone);
  Serial.print(" Time: ");
  formatDate(envirData->time,dateTimeStr );
  Serial.print(dateTimeStr);
  Serial.println();
}
void addBulkData(BulkData * bulk, Environment * toAdd){
	bulk->data[bulk->pointer].novaPm25 = toAdd->novaPm25;
	bulk->data[bulk->pointer].time = toAdd->time;
	
	//move pointer to the next available slot
	bulk->pointer = bulk->pointer + 1;
	//if pointer exit the capacity, start from 0
	if (bulk->pointer == BULK_CAPACITY){
		bulk->pointer = 0;
	}
	if (bulk->bulkCount < BULK_CAPACITY){
		bulk->bulkCount += 1;
	}
}
void printBulkData(BulkData * bulk){
	char dateTimeStr[25];
  Serial.println("====");
  Serial.print("Count: " );
  Serial.println(bulk->bulkCount);
  for(int i = 0;  i < bulk->bulkCount; i++){
    formatDate(bulk->data[i].time,dateTimeStr );
	
	Serial.print("Raw time: ");
	Serial.print(bulk->data[i].time);
	Serial.print("\t Formatted: ");
    Serial.println(dateTimeStr);
  }
  Serial.println("====");
}
void formatDate(long timeSeconds, char * dateStr){
	sprintf(dateStr, "%i-%i-%iT%i:%i:%i.000Z", year(timeSeconds)
											 , month(timeSeconds)
											 , day(timeSeconds)
											 , hour(timeSeconds)
											 , minute(timeSeconds)
											 , second(timeSeconds)
											 );
}
void saveBulkData(BulkData * bulk, int index){
  EEPROM.put(index,*bulk);
  EEPROM.commit();
  delay(50);
}
void loadBulkData(BulkData * bulk, int index){
  EEPROM.get(index,*bulk);
  
  if (bulk->bulkCount > BULK_CAPACITY || bulk->bulkCount < 0){
    bulk->bulkCount = 0;
    bulk->pointer = 0;
  }
}
	