#ifndef _display_
#define _display_

#include <iot_iconset_16x16.h>
#include "timeNTP.h"
#include <IotWebConf.h>
#include "emon.h"
#include "influxdb.h"
#include <Wire.h>  
#include "HT_SSD1306Wire.h"

/*
#if BOARD_USED == heltec_wifi_kit_32_v3
#define SwitchLeft 41
#define SwitchRight 40
#define StatusLED 35
#elif BOARD_USED == heltec_wifi_kit_32_v2
*/
#define SwitchLeft 23
#define SwitchRight 18
#define StatusLED 25
//#endif


class displayClass
{
private:
    // Actual Display Number
    uint8_t ActualDisplay = 4;

    SSD1306Wire *display; // addr , freq , i2c group , resolution , rst

    void ShowWifiDisplay();
    void ShowTimeDisplay();
    void ShowSolarPower();
    void ShowSolarAmp();
    void ShowInfluxDisplay();

    void VextON(void);
    void VextOFF(void);

    void sleep();
    void wakeup();

    unsigned long SwitchpreviousMillis = 0;
    bool ButtonPressed = false;
    bool TestMsgSend = false;

    bool ClearDisplay = false;
    int DisplayPauseCounter;

    timeNTP *timentp;
    emon *emonmon;
    influxdbConClass *influxdb;

public:
    displayClass();
    void setup(timeNTP *_timentp, emon *_emon,influxdbConClass *_influxdb);
    void ChangeDisplay();
    void CheckSwitchState();
};

#endif