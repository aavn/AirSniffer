#ifndef _REST_PROPERTY_H
#define _REST_PROPERTY_H
#include <SPI.h>
//#define SNIFFER_TEST true

#define serverAddress_test "192.168.70.91"
#define snifferUrl_test "/api/pollutantvalues"
#define serverPort_test 8080
/*#ifdef SNIFFER_TEST
	#define serverAddress "192.168.70.91"
	#define snifferUrl "/api/pollutantvalues"
	#define finger "BE:3B:D0:15:B9:6F:8C:C4:64:3F:0A:B6:18:70:56:3F:99:AD:6C:0E"
	#define serverPort 8080
#else*/
	#define serverAddress "crowdsniffing.org"
	#define snifferUrl "/api/pollutantvalues"
	#define finger "BE:3B:D0:15:B9:6F:8C:C4:64:3F:0A:B6:18:70:56:3F:99:AD:6C:0E"
	#define serverPort 443
//#endif



struct RestProperty{
  String sender_pro;
  String latitude_pro;
  String longitude_pro;
  String TEMP_SENSOR_pro;
  String PM_SENSOR_pro;
  String mac_str_pro;
};


#endif
