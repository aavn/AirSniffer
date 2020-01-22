
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif
#include <SPI.h>
#include "Sniffer_Dust_Sensor.h"



SnifferDustSensor::SnifferDustSensor(void) {
}

 
void SnifferDustSensor::begin(uint8_t pin_rx, uint8_t pin_tx) {

	  _pin_rx = pin_rx;
	  _pin_tx = pin_tx;
  
	SoftwareSerial *softSerial = new SoftwareSerial(_pin_rx, _pin_tx);
	softSerial->begin(9600);

	novaSerial = softSerial;
	sensor = new Sds011(*novaSerial);
	sensor->set_sleep(false);
    sensor->set_mode(sds011::QUERY);
}

 
bool SnifferDustSensor::readDustData(){
	envData.novaPm10 = 0;
	envData.novaPm25 = 0;
	bool ok = true;
	//for ESP
    //novaSerial->listen();
    sensor->set_sleep(false);
//     delay(5000);
    //read dust sensor value
    unsigned long int lastAttempt = millis();
    int attemp = 0;
    ok = calcNovaPM();
    //If data error or we've tried already 20 times, stop
    while((envData.novaPm25 < 0 || envData.novaPm10 < 0 || envData.novaPm25 >= 300 || envData.novaPm10 >=1000) && attemp++<20) {
      if ((lastAttempt-millis())>5000) {
      lastAttempt = millis();
      	Serial.println("Nova sensor doesn't work properly");
      	ok = calcNovaPM();
      }
//       delay(3000);
    }
    Serial.println("Stop NOVA");
    sensor->set_sleep(true);
	
	return ok;
  }
  
  
//read PM2.5, PM 10 from Nova sensor
bool SnifferDustSensor::calcNovaPM(){
  //Serial.println("calcNovaPM()");
    int pm25, pm10;
    bool ok = true;

//     sensor->set_sleep(false);
//     delay(1000);
    ok = sensor->query_data_auto(&pm25, &pm10, SAMPLES);
    envData.novaPm25 = pm25/10.0;
    envData.novaPm10 = pm10/10.0;
   // if(ok){
   //   digitalWrite(SENSOR_ERR,LOW);
   // }else{
   //   digitalWrite(SENSOR_ERR,HIGH);
    //}
//     Serial.println("Stop NOVA");
// 	sensor->set_sleep(true);
	return ok;
}

float SnifferDustSensor::getPM10(){
	return envData.novaPm10;
}

float SnifferDustSensor::getPM25(){
	
	return envData.novaPm25;
}
