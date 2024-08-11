#include "display.h"

displayClass::displayClass() {};

void displayClass::setup(timeNTP *_timentp, emon *_emon, influxdbConClass *_influxdb)
{
    timentp = _timentp;
    emonmon = _emon;
    influxdb = _influxdb;

    display = new SSD1306Wire(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst

    wakeup();

    pinMode(SwitchLeft, INPUT);
    pinMode(SwitchRight, INPUT);
    pinMode(StatusLED, OUTPUT);
}

void displayClass::ChangeDisplay()
{
    DisplayPauseCounter++;
    if (DisplayPauseCounter >= 75 && !ClearDisplay)
    {
        ClearDisplay = true;
        DisplayPauseCounter = 75;
        sleep();
    }

    if (!ClearDisplay)
    {
        display->clear();
        display->drawString(86, 0, timentp->getFormattedTime());
        display->drawString(122, 54, String(ActualDisplay + 1));
        if (WiFi.status() == WL_CONNECTED)
        {
            int8_t tmpRSSI = WiFi.RSSI();
            if (tmpRSSI > -40)
            {
                display->drawXbm(-1, -2, 16, 16, signal4_icon16x16);
                // Serial.println("1");
            }
            else if (tmpRSSI >= -60)
            {
                display->drawXbm(-1, -2, 16, 16, signal3_icon16x16);
                // Serial.println("2");
            }
            else if (tmpRSSI >= -80)
            {
                display->drawXbm(-1, -2, 16, 16, signal2_icon16x16);
                // Serial.println("3");
            }
            else
            {
                display->drawXbm(-1, -2, 16, 16, signal1_icon16x16);
                // Serial.println("4");
            }
        }
        else
        {
            display->drawXbm(-1, -2, 16, 16, nocon_icon16x16);
        }
        switch (ActualDisplay)
        {
        case 0:
            ShowWifiDisplay();
            break;
        case 1:
            ShowTimeDisplay();
            break;
        case 2:
            ShowSolarPower();
            break;
        case 3:
            ShowSolarAmp();
            break;
        case 4:
            ShowInfluxDisplay();
            break;
        default:
            break;
        }
        display->display();
    }
}

void displayClass::ShowWifiDisplay()
{
    display->drawString(16, 0, "W-LAN");
    if (WiFi.status() == WL_CONNECTED)
    {
        display->drawString(0, 12, "Verbunden!");
        display->drawString(0, 24, "IP: " + WiFi.localIP().toString());
        display->drawString(0, 36, "RSSI: " + (String)WiFi.RSSI() + "dbm");
    }
    else
    {
        display->drawString(0, 12, "Keine Verbindung!");
    }
    display->drawString(0, 48, String(timentp->getLaufZeitDay()) + " Tage " + String(timentp->getLaufZeitHour()) + "h " + String(timentp->getLaufZeitMin()) + "min");
}

void displayClass::ShowInfluxDisplay()
{
    display->drawString(16, 0, "InfluxDB");
    if (influxdb->getConnection())
    {
        display->drawString(0, 24, "Verbunden!");
    }
    else
    {
        display->drawString(0, 24, "Keine Verbindung!");

        display->drawString(0, 36, "Fehlversuche:");
        display->drawString(0, 48, String(influxdb->getFailAttempts()));
    }
}

void displayClass::ShowTimeDisplay()
{
    display->drawString(16, 0, "Datum/Zeit");
    display->drawString(0, 12, "Tag: " + timentp->getDay());
    display->drawString(0, 24, "Monat: " + timentp->getMonth());
    display->drawString(0, 36, "Datum: " + timentp->getFormattedDate());
    display->drawString(0, 48, "Uhrzeit: " + timentp->getFormattedTime());
}

void displayClass::ShowSolarAmp()
{
    display->drawString(16, 0, "Solar I & P");
    display->drawString(0, 14, "I1: " + String(emonmon->getIrms1()) + "A");
    display->drawString(0, 28, "I2: " + String(emonmon->getIrms2()) + "A");
    display->drawString(0, 42, "I3: " + String(emonmon->getIrms3()) + "A");
    display->drawString(54, 14, "P1: " + String(emonmon->getP1()) + "kW");
    display->drawString(54, 28, "P2: " + String(emonmon->getP2()) + "kW");
    display->drawString(54, 42, "P3: " + String(emonmon->getP3()) + "kW");
}

void displayClass::ShowSolarPower()
{
    display->drawString(16, 0, "Solar-Power");

    display->drawString(0, 12, "Pges: " + String(emonmon->getPGesamt()) + "kW");
    display->drawString(0, 24, "Pmax: " + String(emonmon->getPMax()) + "kW");
    display->drawString(0, 36, "Tag Leistung:");
    display->drawString(0, 48, String(emonmon->getPTag()) + "kWh");
}

void displayClass::CheckSwitchState()
{
    bool ButtonLeftState = digitalRead(SwitchLeft);
    bool ButtonRightState = digitalRead(SwitchRight);

    unsigned long currentMillis = millis();

    if (!ButtonLeftState && !ButtonRightState)
    {
        SwitchpreviousMillis = currentMillis;
        ButtonPressed = false;
    }

    if ((currentMillis - SwitchpreviousMillis >= 50) && (ButtonLeftState || ButtonRightState))
    {
        if (ClearDisplay)
        {
            ClearDisplay = false;
            DisplayPauseCounter = 0;
            wakeup();
        }

        if (!ButtonPressed)
        {
            if (ButtonLeftState)
            {
                ActualDisplay++;
                ActualDisplay = ActualDisplay % 5;
            }
            if (ButtonRightState)
            {
                if (ActualDisplay == 0)
                {
                    ActualDisplay = 4;
                }
                else
                {
                    ActualDisplay--;
                }
            }
            ButtonPressed = true;
        }
    }
}

void displayClass::VextON(void)
{
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
}

void displayClass::VextOFF(void) // Vext default OFF
{
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, HIGH);
}

void displayClass::sleep()
{
    display->clear();
    display->display();
    delay(300);
    VextOFF();
}

void displayClass::wakeup()
{
    VextON();
    delay(100);

    display->init();
    display->clear();
    display->display();

    display->setContrast(255);

    DEBUG_LINE("Wakup Done");
}
