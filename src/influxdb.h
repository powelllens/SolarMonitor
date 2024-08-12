#ifndef _influxdb_
#define _influxdb_

#include <Arduino.h>
#include <InfluxDbClient.h>
#include "influxdbCert.h"
#include "emon.h"
#include "timeNTP.h"

//#if BOARD_USED == heltec_wifi_kit_32_v3
//#include "driver/temp_sensor.h"
//#endif

#ifndef DISABLE_DEBUG_TO_SERIAL
#define DEBUG_LINE(MSG) Serial.println(MSG)
#define DEBUG_LINE_(MSG) Serial.print(MSG)
#else
#define DEBUG_LINE(MSG)
#define DEBUG_LINE_(MSG)
#endif

#define WRITE_PRECISION WritePrecision::S
#define MAX_BATCH_SIZE 10
#define WRITE_BUFFER_SIZE 100

class influxdbConClass
{
private:
    InfluxDBClient client;
    // InfluxDB client instance for InfluxDB 1
    // InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB_NAME);

    emon *emonmon = nullptr;
    timeNTP *timelib = nullptr;

    //temp_sensor_config_t temp_sensor = TSENS_CONFIG_DEFAULT();

    unsigned long updateinterval = 60000;
    unsigned long lastDataTime;
    bool timesynced = false;
    bool initalUpload = true;

    uint8_t connectionfailedcounter;

public:
    influxdbConClass();
    void setup(String INFLUXDB_URL, String INFLUXDB_ORG, String INFLUXDB_BUCKET, String INFLUXDB_TOKEN);
    void loop();
    void setUpdateInterval(uint16_t interval);
    void setupEmon(emon *_emon);
    void setupTime(timeNTP *timelib);
    void setTimeSycned() { timesynced = true; };
    void resetTimeSycned() { timesynced = false; };
    bool getConnection() { return !initalUpload; };
    uint8_t getFailAttempts() { return connectionfailedcounter; };
};

#endif