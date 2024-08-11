#ifndef _WEB_CONFIG_
#define _WEB_CONFIG_

#include <Arduino.h>

// general IotWbConf
#include <IotWebConf.h>

// update server
#include <IotWebConfESP32HTTPUpdateServer.h>

#include <IotWebConfUsing.h>
#include <IotWebConfParameter.h>
#include <IotWebConfTParameter.h>

#ifndef DEBUG_LINE
#ifndef DISABLE_DEBUG_TO_SERIAL
#define DEBUG_LINE(MSG) Serial.println(MSG)
#define DEBUG_LINE_(MSG) Serial.print(MSG)
#else
#define DEBUG_LINE(MSG)
#define DEBUG_LINE_(MSG)
#endif
#endif

#define BUILD_VERSION "001"
// general settings
#define CONFIG_VERSION "002"

// ap and wifi
const char WIFI_AP_SSID[] = "Gerd-Solar-Monitor";
const char WIFI_AP_DEFAULT_PASSWORD[] = "gerdsolarmonitor";

#ifndef STRING_LEN
#define STRING_LEN 128
#endif
#ifndef NUMBER_LEN
#define NUMBER_LEN 32
#endif
/*
#if BOARD_USED == heltec_wifi_kit_32_v3
#define StatusLED 35
#elif BOARD_USED == heltec_wifi_kit_32_v2
*/
#define StatusLED 25
//#endif

class WebConfigClass
{
public:
    WebConfigClass();
    void setup();
    void setconfigSavedCallback(std::function<void()> func);
    void setWifiConnectionCallback(std::function<void()> func);
    void loop();

    iotwebconf::ParameterGroup INFLUXDB_ParamGroup = iotwebconf::ParameterGroup("influxdb_group", "InfluxDB Einstellungen");

    iotwebconf::TextTParameter<STRING_LEN> INFLUXDB_URL_Param =
        iotwebconf::Builder<iotwebconf::TextTParameter<STRING_LEN>>("influxdb_url")
            .label("InfluxDB URL")
            .defaultValue("")
            .build();

    iotwebconf::TextTParameter<STRING_LEN> INFLUXDB_ORG_Param =
        iotwebconf::Builder<iotwebconf::TextTParameter<STRING_LEN>>("influxdb_org")
            .label("InfluxDB Organisation")
            .defaultValue("")
            .build();

    iotwebconf::TextTParameter<STRING_LEN> INFLUXDB_BUCKET_Param =
        iotwebconf::Builder<iotwebconf::TextTParameter<STRING_LEN>>("influxdb_bucket")
            .label("InfluxDB Bucket")
            .defaultValue("")
            .build();

    iotwebconf::TextTParameter<STRING_LEN> INFLUXDB_TOKEN_Param =
        iotwebconf::Builder<iotwebconf::TextTParameter<STRING_LEN>>("influxdb_token")
            .label("InfluxDB Token")
            .defaultValue("")
            .build();

    iotwebconf::IntTParameter<int16_t> INFLUXDB_DATATIME_Param =
        iotwebconf::Builder<iotwebconf::IntTParameter<int16_t>>("influxdb_update")
            .label("Update Rate [s]")
            .defaultValue(60)
            .min(10)
            .max(600)
            .step(10)
            .placeholder("10..600")
            .build();

private:
    DNSServer dnsServer;
    WebServer *server;
    HTTPUpdateServer httpUpdater;
    IotWebConf *WebConf;
    boolean RestartRequired;
    iotwebconf::HtmlFormatProvider HtmlFormatProvider;

    void setupConfig();
    void setupUpdateServer();
    void handleRoot();
    void handleConfig();
};

#endif