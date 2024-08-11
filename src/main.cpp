#include "Arduino.h"

#include "emon.h"
#include "timeNTP.h"
#include "display.h"
#include "webconfig.h"
#include "influxdb.h"

#ifndef DISABLE_DEBUG_TO_SERIAL
#define DEBUG_LINE(MSG) Serial.println(MSG)
#define DEBUG_LINE_(MSG) Serial.print(MSG)
#else
#define DEBUG_LINE(MSG)
#define DEBUG_LINE_(MSG)
#endif

//#include "esp_system.h"
#include "esp_task_wdt.h"
#define WDT_TIMEOUT 60
void InitWatchdog();

emon emon_monitor;
timeNTP time_lib;
displayClass display;
WebConfigClass webconfig;
influxdbConClass influxdbCon;

TaskHandle_t Process_Task_H;
TaskHandle_t WebConfig_Task_H;
TaskHandle_t Display_Task_H;
TaskHandle_t Button_Task_H;
TaskHandle_t Time_Task_H;

void Display_Task(void *pvParameters);
void Button_Task(void *pvParameters);
void Process_Task(void *pvParameters);
void WebConfig_Task(void *pvParameters);
void Time_Task(void *pvParameters);

void configSaved();

void setup()
{
    Serial.begin(UPLOAD_SPEED);
    Serial.flush();
    delay(50);
    DEBUG_LINE_("Serial initial done\r\n");

    webconfig.setconfigSavedCallback(&configSaved);
    webconfig.setup();

    influxdbCon.setup(
        webconfig.INFLUXDB_URL_Param.value(),
        webconfig.INFLUXDB_ORG_Param.value(),
        webconfig.INFLUXDB_BUCKET_Param.value(),
        webconfig.INFLUXDB_TOKEN_Param.value());

    influxdbCon.setUpdateInterval(webconfig.INFLUXDB_DATATIME_Param.value());
    influxdbCon.setupEmon(&emon_monitor);
    influxdbCon.setupTime(&time_lib);

    display.setup(&time_lib, &emon_monitor, &influxdbCon);

    xTaskCreatePinnedToCore(
        Process_Task,
        "Process_Task",
        6144,
        NULL,
        1,
        &Process_Task_H,
        1);

    delay(50);

    xTaskCreatePinnedToCore(
        Display_Task,
        "Display_Task",
        2048,
        NULL,
        1,
        &Display_Task_H,
        1);

    delay(50);

    xTaskCreatePinnedToCore(
        Button_Task,
        "Button_Task",
        2048,
        NULL,
        1,
        &Button_Task_H,
        1);

    delay(50);

    xTaskCreatePinnedToCore(
        Time_Task,
        "Time_Task",
        2048,
        NULL,
        1,
        &Time_Task_H,
        1);

    delay(50);

    xTaskCreatePinnedToCore(
        WebConfig_Task,
        "WebConfig_Task",
        4096,
        NULL,
        2,
        &WebConfig_Task_H,
        0);

    delay(200);

    emon_monitor.setup();
    InitWatchdog();
    DEBUG_LINE("End");
}

void loop()
{
    delay(1000);
}

void Process_Task(void *pvParameters)
{
    const TickType_t xDelay = 400 / portTICK_PERIOD_MS;

    DEBUG_LINE("Process Task Started");
    esp_task_wdt_add(NULL);
    for (;;)
    {
        if (time_lib.getResetState())
        {
            emon_monitor.ResetData();
        }
        influxdbCon.loop();

        delay(xDelay);
        esp_task_wdt_reset();
    }
}

void Display_Task(void *pvParameters)
{
    const TickType_t xDelay = 400 / portTICK_PERIOD_MS;
    DEBUG_LINE("Display Task Started");
    esp_task_wdt_add(NULL);
    for (;;)
    {
        display.ChangeDisplay();
        delay(xDelay);
        esp_task_wdt_reset();
    }
}

void Button_Task(void *pvParameters)
{
    const TickType_t xDelay = 50 / portTICK_PERIOD_MS;
    DEBUG_LINE("Button Task Started");
    esp_task_wdt_add(NULL);
    for (;;)
    {
        display.CheckSwitchState();
        delay(xDelay);
        esp_task_wdt_reset();
    }
}

void Time_Task(void *pvParameters)
{
    const TickType_t xDelayShort = 200 / portTICK_PERIOD_MS;
    DEBUG_LINE("Time Task Started");
    esp_task_wdt_add(NULL);
    for (;;)
    {
        time_lib.NTPTimeUpdate();
        if (time_lib.getTimeSynced())
        {
            influxdbCon.setTimeSycned();
        }
        else
        {
            influxdbCon.resetTimeSycned();
        }
        delay(xDelayShort);
        esp_task_wdt_reset();
    }
}

void WebConfig_Task(void *pvParameters)
{
    const TickType_t xDelay = 10 / portTICK_PERIOD_MS;
    DEBUG_LINE("WebConfig Task Started");
    esp_task_wdt_add(NULL);
    for (;;)
    {
        webconfig.loop();
        delay(xDelay);
        esp_task_wdt_reset();
    }
}

void configSaved()
{
    ESP.restart();
}

void InitWatchdog()
{
  Serial.println("Configuring WDT...");
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  //esp_task_wdt_add(NULL);               //add current thread to WDT watch
}
