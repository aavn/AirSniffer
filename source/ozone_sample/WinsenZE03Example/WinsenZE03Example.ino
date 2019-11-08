/*
  WinsenZE03.h - This library allows you to set and read the ZE03 Winsen Sensor module.
	More information: https://github.com/fega/winsen-ze03-arduino-library
  Created by Fabian Gutierrez <fega.hg@gmail.com>, March 17, 2017.
  MIT.
*/

#include <WinsenZE03.h>
#include <SoftwareSerial.h>

SoftwareSerial gtSerial(D5, D1); // Arduino RX, Arduino TX
WinsenZE03 sensor;


void setup() {
  gtSerial.begin(9600);
  Serial.begin(115200);
  sensor.begin(&gtSerial, O3);
	sensor.setAs(QA);
}
void loop() {
 float ppm = sensor.readManual();
 Serial.print("OZONE VALUE:");
 Serial.println(ppm);
 delay(5000);
}
