#include "timeNTP.h"

timeNTP::timeNTP()
{
    configTzTime(TZ_INFO, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
}

bool timeNTP::NTPTimeUpdate()
{
    unsigned long currentruntime = millis();
    if (currentruntime - previousLaufZeit >= 1000)
    {
        previousLaufZeit = currentruntime;
        ++systemuptime;
        ++timecounter;
        if (timecounter > 59)
        {
            timecounter = 0;
            LaufZeit++;
            LaufZeitDay = LaufZeit / 1440;
            LaufZeitHour = (LaufZeit / 60) - (LaufZeitDay * 24);
            LaufZeitMin = LaufZeit - (LaufZeitDay * 1440) - (LaufZeitHour * 60);
        }
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        initialUpdate = true;
        return false;
    }

    byte TriggerMinute = 0;

    if (initialUpdate)
    {
        initialUpdate = false;
        timeSync(TZ_INFO, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
    }

    if ((RunMin == TriggerMinute) && (UpdateTime == false))
    {
        timeSync(TZ_INFO, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
        UpdateTime = true;
    }
    if (RunMin > TriggerMinute)
    {
        UpdateTime = false;
    }

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        // DEBUG_LINE("Failed to obtain time");
        return false;
    }
    formattedTime = digitstring(timeinfo.tm_hour) + ":" + digitstring(timeinfo.tm_min) + ":" + digitstring(timeinfo.tm_sec);
    formattedDate = digitstring(timeinfo.tm_mday) + "." + digitstring(1 + timeinfo.tm_mon) + "." + String(1900 + timeinfo.tm_year);

    RunHour = timeinfo.tm_hour;
    RunMin = timeinfo.tm_min;

    if (SysStartTime == "")
    {
        SysStartTime = formattedTime;
        SysStartDate = formattedDate;
    }

    switch (timeinfo.tm_wday)
    {
    case 1:
        Day = "Montag";
        break;
    case 2:
        Day = "Dienstag";
        break;
    case 3:
        Day = "Mittwoch";
        break;
    case 4:
        Day = "Donnerstag";
        break;
    case 5:
        Day = "Freitag";
        break;
    case 6:
        Day = "Samstag";
        break;
    case 0:
        Day = "Sonntag";
        break;
    default:
        Day = String(timeinfo.tm_wday);
        break;
    }

    switch (timeinfo.tm_mon)
    {
    case 0:
        Month = "Januar";
        break;
    case 1:
        Month = "Februar";
        break;
    case 2:
        Month = "März";
        break;
    case 3:
        Month = "April";
        break;
    case 4:
        Month = "Mai";
        break;
    case 5:
        Month = "Juni";
        break;
    case 6:
        Month = "Juli";
        break;
    case 7:
        Month = "August";
        break;
    case 8:
        Month = "September";
        break;
    case 9:
        Month = "Oktober";
        break;
    case 10:
        Month = "November";
        break;
    case 11:
        Month = "Dezember";
        break;
    default:
        Month = String(timeinfo.tm_mon);
        break;
    }

    checkResetTime(RunHour, RunMin);

    return true;
}

bool timeNTP::getResetState()
{
    if (ResetData)
    {
        ResetData = false;
        return true;
    }
    return false;
};

void timeNTP::checkResetTime(int Hour, int Minute)
{
    byte TriggerHour = 0;
    byte TriggerMinute = 0;

    if ((Hour == TriggerHour) && (Minute == TriggerMinute) && (ResetDone == false))
    {
        ResetData = true;
        ResetDone = true;
    }
    if ((Hour == TriggerHour) && (Minute > TriggerMinute))
    {
        ResetDone = false;
    }
}

String timeNTP::digitstring(int digit)
{
    String strdigit;
    if (digit < 10)
    {
        strdigit = "0";
    }
    strdigit = strdigit + String(digit);
    return strdigit;
}