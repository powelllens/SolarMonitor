#include "emon.h"

EnergyMonitor emon::emons1; // Create an instance
EnergyMonitor emon::emons2; // Create an instance
EnergyMonitor emon::emons3; // Create an instance

double emon::Irms1, emon::Irms2, emon::Irms3;
double emon::Irms1F, emon::Irms2F, emon::Irms3F;
double emon::P1, emon::P2, emon::P3;
double emon::PGesamt;
double emon::PTag;
double emon::PMax;
uint8_t emon::EmonStartCounter;

double emon::PowerArray[48];
String emon::PowerTimeArray[48];
uint8_t emon::PowerArrayCounter;

TaskHandle_t emon::emon_task_h;
SemaphoreHandle_t emon::sync_emon;

emon::emon() {};

void emon::setup()
{
    // sync_emon = xSemaphoreCreateMutex();
    //  Setup the ADC

    pinMode(ADC_INPUT_A1, INPUT);
    pinMode(ADC_INPUT_A2, INPUT);
    pinMode(ADC_INPUT_A3, INPUT);

    // analogSetAttenuation(ADC_11db);
    analogSetPinAttenuation(ADC_INPUT_A1, ADC_11db);
    analogSetPinAttenuation(ADC_INPUT_A2, ADC_11db);
    analogSetPinAttenuation(ADC_INPUT_A3, ADC_11db);

    analogReadResolution(ADC_BITS);

    emons1.current(ADC_INPUT_A1, 162.0); // Current: input pin, calibration.
    emons2.current(ADC_INPUT_A2, 162.0); // Current: input pin, calibration.
    emons3.current(ADC_INPUT_A3, 162.0); // Current: input pin, calibration.

    xTaskCreatePinnedToCore(
        emon_task,
        "taskemon",
        2048,
        NULL,
        2,
        &emon_task_h,
        1);
};

void emon::emon_task(void *pvParameters) // This is a task.
{
    (void)pvParameters;

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 1000;
    BaseType_t xWasDelayed;

    Serial.println("start emon");
    // esp_task_wdt_add(emon_task_h);
    xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        xWasDelayed = xTaskDelayUntil(&xLastWakeTime, xFrequency);

        // Default 1480
        // https://community.openenergymonitor.org/t/sampling-rate-of-emonlib/4383

        // https://community.openenergymonitor.org/t/emonlib-esp32-how-to-calibrate/11465

        Irms1 = IrmsZero(emons1.calcIrms(1745)); // Calculate Irms only
        Irms2 = IrmsZero(emons2.calcIrms(1745)); // Calculate Irms only
        Irms3 = IrmsZero(emons3.calcIrms(1745)); // Calculate Irms only

        Irms1F = (Irms1F + Irms1) / 2.0;
        Irms2F = (Irms2F + Irms2) / 2.0;
        Irms3F = (Irms3F + Irms3) / 2.0;

        P1 = (Irms1F * 230.0) / 1000.0; // Apparent power
        P2 = (Irms2F * 230.0) / 1000.0; // Apparent power
        P3 = (Irms3F * 230.0) / 1000.0; // Apparent power

        EmonStartCounter++;
        if (EmonStartCounter >= 20)
        {
            EmonStartCounter = 20;
            PGesamt = P1 + P2 + P3;
            if (PMax < PGesamt)
            {
                PMax = PGesamt;
            }

            PTag = PTag + (PGesamt / 3600);
        }
        // esp_task_wdt_reset();
    }
}

void emon::ResetData()
{
    PTag = 0;
    PMax = 0;
    PGesamt = 0;
}

double emon::IrmsZero(double input)
{
    if (input < 1.0)
    {
        input = 0.0;
    }
    return input;
}

/*
    #ifndef EMON_IGNORE_THREADSAFETY
        xSemaphoreTake(sync_emon, portMAX_DELAY);
#endif
        uint8_t tmp_dmx = dmx_data[channel];
#ifndef EMON_IGNORE_THREADSAFETY
        xSemaphoreGive(sync_serial_dmx);
#endif
*/