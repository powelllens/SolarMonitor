#ifndef _timentp_
#define _timentp_

#include "Arduino.h"
#include "time.h"
#include <WiFi.h>
#include <InfluxDbClient.h>

#ifndef DISABLE_DEBUG_TO_SERIAL
#define DEBUG_LINE(MSG) Serial.println(MSG)
#define DEBUG_LINE_(MSG) Serial.print(MSG)
#else
#define DEBUG_LINE(MSG)
#define DEBUG_LINE_(MSG)
#endif

#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

#define NTP_SERVER1 "time.nist.gov"
#define NTP_SERVER2 "0.pool.ntp.org"
#define NTP_SERVER3 "1.pool.ntp.org"

class timeNTP
{
private:
    uint8_t timecounter;
    unsigned long long systemuptime = 0;

    const long gmtOffset_sec = 3600;
    const int daylightOffset_sec = 0;

    String formattedTime = "No Time";
    String formattedDate = "No Date";

    String SysStartTime = "";
    String SysStartDate = "";

    void checkResetTime(int Hour, int Minute);
    // void printLocalTime();
    // String PowerArrayToString(byte ArrayNumber);
    String digitstring(int digit);
    // struct tm GetTime();

    String Day;
    String Month;

    unsigned long LaufZeit = 0;
    unsigned long previousLaufZeit = 0;
    int LaufZeitDay = 0;
    int LaufZeitHour = 0;
    int LaufZeitMin = 0;

    int RunHour;
    int RunMin;

    bool ResetDone;
    bool ResetData;
    bool UpdateTime;

    bool initialUpdate = true;

public:
    timeNTP();

    bool NTPTimeUpdate();
    bool getResetState();
    String getFormattedTime() { return formattedTime; };
    String getFormattedDate() { return formattedDate; };
    int getLaufZeitDay() { return LaufZeitDay; };
    int getLaufZeitHour() { return LaufZeitHour; };
    int getLaufZeitMin() { return LaufZeitMin; };
    String getDay() { return Day; };
    String getMonth() { return Month; };
    bool getTimeSynced() { return !initialUpdate; };
    unsigned long long getUptime() { return systemuptime; };
};

#endif