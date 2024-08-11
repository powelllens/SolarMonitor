#include "webconfig.h"

WebConfigClass::WebConfigClass()
{
    this->server = new WebServer(80);
    this->WebConf = new IotWebConf(WIFI_AP_SSID, &dnsServer, server, WIFI_AP_DEFAULT_PASSWORD, CONFIG_VERSION);
}

void WebConfigClass::setWifiConnectionCallback(std::function<void()> func)
{
    this->WebConf->setWifiConnectionCallback(func);
}

void WebConfigClass::setup()
{
    this->setupConfig();
    this->setupUpdateServer();
    this->WebConf->skipApStartup();
    this->WebConf->setStatusPin(StatusLED, 1);
    this->WebConf->init();

    // -- Set up required URL handlers on the web server.
    this->server->on("/", [&]
                     { handleRoot(); });
    this->server->on("/config", [&]
                     { this->WebConf->handleConfig(); });
    this->server->onNotFound([&]()
                             { this->WebConf->handleNotFound(); });
}

void WebConfigClass::loop()
{
    disableCore0WDT(); // Bugfix -> Dirty Workaround... if the Config is to big it is very not non blocking, therefore if the iotwebconf class is used in a seperate thread it may cause a IDLE Task timeout ToDo: improve to a more non blocking bugfix, in the meantime we disable the watchdog while in the wifi loop
    this->WebConf->doLoop();
    enableCore0WDT(); // Enable the watchdog as soon as finished with the wifi loop
  
}

void WebConfigClass::setupConfig()
{
    // Config Parameters

    this->INFLUXDB_ParamGroup.addItem(&INFLUXDB_URL_Param);
    this->INFLUXDB_ParamGroup.addItem(&INFLUXDB_ORG_Param);
    this->INFLUXDB_ParamGroup.addItem(&INFLUXDB_BUCKET_Param);
    this->INFLUXDB_ParamGroup.addItem(&INFLUXDB_TOKEN_Param);
    this->INFLUXDB_ParamGroup.addItem(&INFLUXDB_DATATIME_Param);

    this->WebConf->addSystemParameter(&INFLUXDB_ParamGroup);
}

void WebConfigClass::setupUpdateServer()
{
    this->WebConf->setupUpdateServer(
        [&](const char *updatePath)
        { httpUpdater.setup(server, updatePath); },
        [&](const char *userName, char *password)
        { httpUpdater.updateCredentials(userName, password); });
}

void WebConfigClass::setconfigSavedCallback(std::function<void()> func)
{
    this->WebConf->setConfigSavedCallback(func);
}

void WebConfigClass::handleRoot()
{
    // -- Let IotWebConf test and handle captive portal requests.
    if (this->WebConf->handleCaptivePortal())
    {
        // -- Captive portal request were already served.
        return;
    }
    int8_t tmpWifiStrenght = WiFi.RSSI();
    String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\" />";
    s += "<title>Actual Status Overview</title>\n";
    s += "<style>";
    s += FPSTR(IOTWEBCONF_HTML_STYLE_INNER);
    s += "</style>\n</head><body>\n";
    s += "<div style=\"text-align:left;display:inline-block;min-width:260px;\">\n<br><fieldset><legend>";
    s += String(this->WebConf->getThingName());
    s += "</legend>";
    s += "<fieldset><legend>WiFi</legend>";
    if (tmpWifiStrenght > -30)
    {
        s += "Amazing";
    }
    else if (tmpWifiStrenght > -55)
    {
        s += "Very good signal";
    }
    else if (tmpWifiStrenght > -67)
    {
        s += "Fairly Good";
    }
    else if (tmpWifiStrenght > -70)
    {
        s += "Okay";
    }
    else if (tmpWifiStrenght > -80)
    {
        s += "Not Good";
    }
    else if (tmpWifiStrenght > -90)
    {
        s += "Extremely weak signal";
    };
    s += "<br>WiFi Signal RSSI: ";
    s += String(tmpWifiStrenght);
    s += "dBm";
    s += "<legend></fieldset><br>";
     s += "<div>Go to <a href='config'>configure page</a> to change values.</div>";
    s += "</div></body></html>\n";

    this->server->send(200, "text/html", s);
}