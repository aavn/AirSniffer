#include "Sniffer_Rest_Property.h"

#include "Sniffer_Data_Util.h"

#include <SPI.h>


Environment envData;
#define sender "nJn1GBek70"
#define latitude "10.833787"
#define longitude "106.60434"

#define TEMP_SENSOR "DHT11"
#define PM_SENSOR "Nova fitness"


char data[600];
RestProperty restProperty;
void setup() {
  //setup wifi

  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  

  
  // put your setup code here, to run once:
   envData.novaPm25 = 10.0;
   envData.novaPm10 = 20.0;
   envData.temperature = 25.0;
   envData.humidity = 45.0;
   printData(&envData);
   restProperty.sender_pro= sender;
    restProperty.latitude_pro= latitude;
    restProperty.longitude_pro= longitude;
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
