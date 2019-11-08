#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#define SOURCE_KEY "source"
#define SOURCE_ID_KEY "sourceId"
#define MAC_KEY "macAddress"
#define GPS_KEY "gpsLocation"
#define LAT_KEY "latitude"
#define LONG_KEY "longitude"


#define VALS_KEY "values"
#define VAL_KEY "value"
#define MEASURE_KEY "measuredAt"

#define CODE_KEY "code"
#define SENSOR_KEY "sensor"

#define OZONE_KEY "O3"
#define PM25_KEY "PM2.5"
#define PM10_KEY "PM10"
#define HUM_KEY "HUM"
#define TEMP_KEY "TEMP"

/* Set these to your desired credentials. */
//#define defaultSSID "ESPap"
//#define defaultPWD "thereisnospoon"

#define STRING_SIZE 80
#define TEMP_MAX 100
#define TEMP_MIN 0
#define HUM_MAX 100
#define HUM_MIN 0
#define PM_MAX 999.9
#define PM_MIN 0.0
#define OZONE_MIN 0
#define OZONE_MAX 10000 //ppb

const char degree = char (223);
const char micro = char (241);

struct RestProperty{
  String sender_pro;
  String latitude_pro;
  String longitude_pro;
  String TEMP_SENSOR_pro;
  String PM_SENSOR_pro;
  String mac_str_pro;
  String OZONE_SENSOR_pro;
};


#define CUBIC_INDEX 0
#define DATA_SIZE 800
#endif
