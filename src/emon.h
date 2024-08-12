#ifndef _emon_
#define _emon_

#include <EmonLib.h>
#include <esp_task_wdt.h>

#define EMON_IGNORE_THREADSAFETY 0 // set to 1 to disable all threadsafe mechanisms
/*
#if BOARD_USED == heltec_wifi_kit_32_v3
#define ADC_INPUT_A1 1
#define ADC_INPUT_A2 2
#define ADC_INPUT_A3 3
#elif BOARD_USED == heltec_wifi_kit_32_v2
*/
#define ADC_INPUT_A1 36
#define ADC_INPUT_A2 37
#define ADC_INPUT_A3 38
//#endif

class emon
{
private:
    static EnergyMonitor emons1; // Create an instance
    static EnergyMonitor emons2; // Create an instance
    static EnergyMonitor emons3; // Create an instance

    static double Irms1, Irms2, Irms3;
    static double Irms1F, Irms2F, Irms3F;
    static double P1, P2, P3;
    static double PGesamt;
    static double PTag;
    static double PMax;
    static byte EmonStartCounter;

    static double PowerArray[48];
    static String PowerTimeArray[48];
    static byte PowerArrayCounter;

    static void emon_task(void *pvParameters);
    static TaskHandle_t emon_task_h;

public:
    emon();
    void setup();
    static void ResetData();
    static double IrmsZero(double input);

    static double getP1() { return P1; };
    static double getP2() { return P2; };
    static double getP3() { return P3; };

    static double getIrms1() { return Irms1; };
    static double getIrms2() { return Irms2; };
    static double getIrms3() { return Irms3; };

    static double getPGesamt() { return PGesamt; };
    static double getPMax() { return PMax; };
    static double getPTag() { return PTag; };
};

#endif