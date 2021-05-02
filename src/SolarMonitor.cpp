#include "Arduino.h"
//Include Settings File
#include "SolarMonitorSettings.h"

//**************************************** DISPLAY ************************************************
#include "heltec.h"
#include "iot_iconset_16x16.h"

// Actual Display Number
byte ActualDisplay = 0;

void InitDisplay();
void ChangeDisplay();
void ShowWifiDisplay();
void ShowTimeDisplay();
void ShowSolarPower();
void ShowSolarAmp();
void ShowTest();

bool ClearDisplay = false;
int DisplayPauseCounter;

//**************************************** WIFI ************************************************
#include <WiFi.h>

// Wifi Settings
//const char* ssid = "Defined in SolarMonitorSettings.h";
//const char* password = "Defined in SolarMonitorSettings.h";

void WiFiEvent(WiFiEvent_t event);
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void WIFISetup();
void CheckConnection();

int WifiReconnectionAttempts = 0;
bool WifiRebootConnected = false;
bool WifiNetworkAvaliable = false;

void keepWiFiAlive(void *parameter);

//**************************************** HTTPS ************************************************
// HTTP POST infos für SLACK

#include <HTTPClient.h>

bool SendSlackMsg(String URL, String Message);
void PushSlackMsg(String Msg, bool Info);

// Used for Slack Messages if Wifi Connection is established
// Domain Name with full URL Path for HTTPS POST Request
//const char* SlackStromerzeugung = "Defined in SolarMonitorSettings.h";
//const char* SlackSysteminfos = "Defined in SolarMonitorSettings.h";

bool SlackMsgAvaliable[5];
byte SlackMsgCounter;
String SlackMsg[5];
bool SlackInfo[5];
bool ReportSend = false;
bool TookMeasurement = false;

//**************************************** NTP ************************************************
//NTP
#include "time.h"

const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 0;

String formattedTime = "No Time";
String formattedDate = "No Date";

String SysStartTime = "";
String SysStartDate = "";

void NTPTimeUpdate();
void printLocalTime();
void InitNTPTime();
String PowerArrayToString(byte ArrayNumber);
String digitstring(int digit);
struct tm GetTime();

String Day;
String Month;

bool NTPOK = false;
byte FailedNTPAttempts = 0;
bool NTPFail = false;
unsigned long LaufZeit = 0;
unsigned long previousLaufZeit = 0;
int LaufZeitDay = 0;
int LaufZeitHour = 0;
int LaufZeitMin = 0;

int RunHour;
int RunMin;

const String Laufzeitkleiner24h = "Achtung Systemlaufzeit kürzer als 24h - Messfehler möglich!";
const String Laufzeithoeher24h = "Alle Systeme laufen normal!";

//**************************************** WATCHDOG ************************************************
#include "esp_system.h"

//**************************************** SMTP MAIL ************************************************
#include "Arduino.h"
#include <EMailSender.h>

// To send Email using Gmail use port 465 (SSL) and SMTP Server smtp.gmail.com
// YOU MUST ENABLE less secure app option https://myaccount.google.com/lesssecureapps?pli=1
//#define AUTHOR_EMAIL    "Defined in SolarMonitorSettings.h"
//#define AUTHOR_PASSWORD "Defined in SolarMonitorSettings.h"
//#define emailSubject    "Defined in SolarMonitorSettings.h"

EMailSender emailSend(AUTHOR_EMAIL, AUTHOR_PASSWORD, "Messsystem Sender", emailSubject);

//const char* emailRecipient = "Defined in SolarMonitorSettings.h";
//const char* emailNameRecipient = "Defined in SolarMonitorSettings.h";

double MailPMax = 0;
double MailPTag = 0;

void SendSMTPMail();
void CheckToSendMail();
void CheckToSendMsg();
void SendTestMsg();

bool SendMail = false;

byte FailedMailCounter = 0;

//**************************************** EMON LIB ************************************************
// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3

#include "EmonLib.h" // Include Emon Library

#define ADC_INPUT_A1 36
#define ADC_INPUT_A2 37
#define ADC_INPUT_A3 38

EnergyMonitor emons1; // Create an instance
EnergyMonitor emons2; // Create an instance
EnergyMonitor emons3; // Create an instance

double Irms1, Irms2, Irms3;
double Irms1F, Irms2F, Irms3F;
double P1, P2, P3;
double PGesamt;
double PTag;
double PMax;
byte EmonStartCounter = 0;

void InitCurrents();
void CalculatePower();
double IrmsZero(double input);

double PowerArray[48];
String PowerTimeArray[48];
byte PowerArrayCounter;

//**************************************** RESET REASON ************************************************
#include <rom/rtc.h>
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */

String print_reset_reason(RESET_REASON reason);

//**************************************** Task Core 0 ************************************************

// define two tasks for Blink & AnalogRead
void TaskEthernet(void *pvParameters);
void TaskLocal(void *pvParameters);

//**************************************** Button Inputs ************************************************

#define SwitchLeft 23
#define SwitchRight 18

#define StatusLED 25

unsigned long SwitchpreviousMillis = 0;
bool ButtonPressed = false;
bool TestMsgSend = false;

void InitSwitch();
void CheckSwitchState();

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskWifi(void *pvParameters) // This is a task.
{
  (void)pvParameters;
  Serial.println("Start Wifi");
  WIFISetup();
  for (;;)
  {
    CheckConnection();
    vTaskDelay(10000);
    //    Serial.print("Wifi");
    //    Serial.println(uxTaskGetStackHighWaterMark( NULL ));
  }
}

void TaskMsg(void *pvParameters) // This is a task.
{
  (void)pvParameters;
  Serial.println("Start Msg");
  for (;;) // A Task shall never return or exit.
  {
    CheckToSendMsg();
    CheckToSendMail();

    if (TestMsgSend)
    {
      Serial.println("Test Triggered Send Msg");
      SendTestMsg();
      TestMsgSend = false;
    }
    vTaskDelay(5000);
    digitalWrite(StatusLED, HIGH);
    vTaskDelay(100);
    digitalWrite(StatusLED, LOW);
    //    Serial.print("MSG");
    //    Serial.println(uxTaskGetStackHighWaterMark( NULL ));
  }
}

void TaskTime(void *pvParameters) // This is a task.
{
  (void)pvParameters;
  Serial.println("Start Time");
  // Intit Task
  InitNTPTime();

  for (;;) // A Task shall never return or exit.
  {
    NTPTimeUpdate();
    vTaskDelay(800);
    //    Serial.print("NTP");
    //    Serial.println(uxTaskGetStackHighWaterMark( NULL ));
  }
}

void TaskDisplay(void *pvParameters) // This is a task.
{
  (void)pvParameters;
  Serial.println("Start Display");
  // Intit Task
  InitDisplay();

  for (;;)
  {
    ChangeDisplay();
    vTaskDelay(200); // one tick delay (15ms) in between reads for stability
    //    Serial.print("Disp");
    //    Serial.println(uxTaskGetStackHighWaterMark( NULL ));
  }
}

void TaskEmon(void *pvParameters) // This is a task.
{
  (void)pvParameters;
  Serial.println("Start Emon");
  InitCurrents();

  for (;;)
  {
    long start = millis();
    CalculatePower();
    long end = millis();

    // Schedule the task to run again in 1 second (while
    // taking into account how long measurement took)
    vTaskDelay((1000 - (end - start)) / portTICK_PERIOD_MS);
    //    Serial.print("Emon");
    //    Serial.println(uxTaskGetStackHighWaterMark( NULL ));
  }
}

void TaskSwitch(void *pvParameters) // This is a task.
{
  (void)pvParameters;
  Serial.println("Start Switch");
  // Intit Task
  InitSwitch();

  for (;;)
  {
    CheckSwitchState();
    vTaskDelay(50); // one tick delay (15ms) in between reads for stability
    //    Serial.print("switch");
    //    Serial.println(uxTaskGetStackHighWaterMark( NULL ));
  }
}

//**************************************** DISPLAY ************************************************
void InitDisplay()
{
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, true /*Serial Enable*/);
  delay(300);
  Heltec.display->clear();
}

void ChangeDisplay()
{
  DisplayPauseCounter++;
  if (DisplayPauseCounter >= 1000)
  {
    ClearDisplay = true;
    DisplayPauseCounter = 1000;
  }

  Heltec.display->clear();
  if (!ClearDisplay)
  {
    Heltec.display->drawString(86, 0, formattedTime);
    Heltec.display->drawString(122, 54, String(ActualDisplay + 1));
    if (WifiNetworkAvaliable)
    {
      int8_t tmpRSSI = WiFi.RSSI();
      if (tmpRSSI > -40)
      {
        Heltec.display->drawXbm(-1, -2, 16, 16, signal4_icon16x16);
        //Serial.println("1");
      }
      else if (tmpRSSI >= -60)
      {
        Heltec.display->drawXbm(-1, -2, 16, 16, signal3_icon16x16);
        //Serial.println("2");
      }
      else if (tmpRSSI >= -80)
      {
        Heltec.display->drawXbm(-1, -2, 16, 16, signal2_icon16x16);
        //Serial.println("3");
      }
      else
      {
        Heltec.display->drawXbm(-1, -2, 16, 16, signal1_icon16x16);
        //Serial.println("4");
      }
    }
    else
    {
      Heltec.display->drawXbm(-1, -2, 16, 16, nocon_icon16x16);
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
      ShowTest();
      break;
    default:
      break;
    }
  }
  Heltec.display->display();
}

void ShowWifiDisplay()
{
  Heltec.display->drawString(16, 0, "W-LAN");
  if (WifiNetworkAvaliable)
  {
    Heltec.display->drawString(0, 12, "Verbunden!");
    Heltec.display->drawString(0, 24, "IP: " + WiFi.localIP().toString());
    Heltec.display->drawString(0, 36, "RSSI: " + (String)WiFi.RSSI() + "dbm");
  }
  else
  {
    Heltec.display->drawString(0, 12, "Keine Verbindung!");
    //Heltec.display -> drawString(0, 24, "Letzter Versuch - " + String((millis() - previousMillis) / 1000) + "sek");
    Heltec.display->drawString(0, 36, "Fehlversuche: " + String(WifiReconnectionAttempts));
  }
  Heltec.display->drawString(0, 48, String(LaufZeitDay) + " Tage " + String(LaufZeitHour) + "h " + String(LaufZeitMin) + "min");
}

void ShowTest()
{
  Heltec.display->drawString(16, 0, "Test");
  if (WifiNetworkAvaliable)
  {
    Heltec.display->drawString(0, 12, "Test gestartet");
  }
  Heltec.display->drawString(0, 48, String(LaufZeitDay) + " Tage " + String(LaufZeitHour) + "h " + String(LaufZeitMin) + "min");
}

void ShowTimeDisplay()
{
  Heltec.display->drawString(16, 0, "Datum/Zeit");
  Heltec.display->drawString(0, 12, "Tag: " + Day);
  Heltec.display->drawString(0, 24, "Monat: " + Month);
  Heltec.display->drawString(0, 36, "Datum: " + formattedDate);
  Heltec.display->drawString(0, 48, "Uhrzeit: " + formattedTime);
}

void ShowSolarAmp()
{
  Heltec.display->drawString(16, 0, "Solar I & P");
  Heltec.display->drawString(0, 14, "I1: " + String(Irms1) + "A");
  Heltec.display->drawString(0, 28, "I2: " + String(Irms2) + "A");
  Heltec.display->drawString(0, 42, "I3: " + String(Irms3) + "A");
  Heltec.display->drawString(54, 14, "P1: " + String(P1) + "kW");
  Heltec.display->drawString(54, 28, "P2: " + String(P2) + "kW");
  Heltec.display->drawString(54, 42, "P3: " + String(P3) + "kW");
}

void ShowSolarPower()
{
  Heltec.display->drawString(16, 0, "Solar-Power");

  Heltec.display->drawString(0, 12, "Pges: " + String(PGesamt) + "kW");
  Heltec.display->drawString(0, 24, "Pmax: " + String(PMax) + "kW");
  Heltec.display->drawString(0, 36, "Tag Leistung:");
  Heltec.display->drawString(0, 48, String(PTag) + "kWh");
}

//**************************************** WIFI ************************************************
void WiFiEvent(WiFiEvent_t event)
{
  Serial.printf("[WiFi-event] event: %d\n", event);

  switch (event)
  {
  case SYSTEM_EVENT_WIFI_READY:
    Serial.println(F("WiFi interface ready"));
    break;
  case SYSTEM_EVENT_SCAN_DONE:
    Serial.println(F("Completed scan for access points"));
    break;
  case SYSTEM_EVENT_STA_START:
    Serial.println(F("WiFi client started"));
    break;
  case SYSTEM_EVENT_STA_STOP:
    Serial.println(F("WiFi clients stopped"));
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
    Serial.println(F("Connected to access point"));
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println(F("Disconnected from WiFi access point"));
    WifiNetworkAvaliable = false;
    break;
  case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
    Serial.println(F("Authentication mode of access point has changed"));
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.print(F("Obtained IP address: "));
    Serial.println(WiFi.localIP());
    break;
  case SYSTEM_EVENT_STA_LOST_IP:
    Serial.println(F("Lost IP address and IP address is reset to 0"));
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    Serial.println(F("WiFi Protected Setup (WPS): succeeded in enrollee mode"));
    break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:
    Serial.println(F("WiFi Protected Setup (WPS): failed in enrollee mode"));
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    Serial.println(F("WiFi Protected Setup (WPS): timeout in enrollee mode"));
    break;
  case SYSTEM_EVENT_STA_WPS_ER_PIN:
    Serial.println(F("WiFi Protected Setup (WPS): pin code in enrollee mode"));
    break;
  case SYSTEM_EVENT_AP_START:
    Serial.println(F("WiFi access point started"));
    break;
  case SYSTEM_EVENT_AP_STOP:
    Serial.println(F("WiFi access point  stopped"));
    break;
  case SYSTEM_EVENT_AP_STACONNECTED:
    Serial.println(F("Client connected"));
    break;
  case SYSTEM_EVENT_AP_STADISCONNECTED:
    Serial.println(F("Client disconnected"));
    break;
  case SYSTEM_EVENT_AP_STAIPASSIGNED:
    Serial.println(F("Assigned IP address to client"));
    break;
  case SYSTEM_EVENT_AP_PROBEREQRECVED:
    Serial.println(F("Received probe request"));
    break;
  case SYSTEM_EVENT_GOT_IP6:
    Serial.println(F("IPv6 is preferred"));
    break;
  case SYSTEM_EVENT_ETH_START:
    Serial.println(F("Ethernet started"));
    break;
  case SYSTEM_EVENT_ETH_STOP:
    Serial.println(F("Ethernet stopped"));
    break;
  case SYSTEM_EVENT_ETH_CONNECTED:
    Serial.println(F("Ethernet connected"));
    break;
  case SYSTEM_EVENT_ETH_DISCONNECTED:
    Serial.println(F("Ethernet disconnected"));
    break;
  case SYSTEM_EVENT_ETH_GOT_IP:
    Serial.println(F("Obtained IP address"));
    break;
  default:
    break;
  }
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  WifiReconnectionAttempts = 0;
  if (WifiRebootConnected)
  {
    PushSlackMsg(
        "{"
        "\"blocks\": ["
        "{"
        "\"type\": \"header\","
        "\"text\": {"
        "\"type\": \"plain_text\","
        "\"text\": \"W-Lan - Verbindung erneut hergestellt\","
        "\"emoji\": true"
        "}"
        "},"
        "{"
        "\"type\": \"section\","
        "\"text\": {"
        "\"type\": \"mrkdwn\","
        "\"text\": \"Wlan verbunden!\""
        "}"
        "},"
        " {"
        "\"type\": \"divider\""
        "},"
        "{"
        "\"type\": \"section\","
        "\"text\": {"
        "\"type\": \"mrkdwn\","
        "\"text\": \"W-Lan:\nSignalstärke: *" +
            (String)WiFi.RSSI() +
            "dbm*\nErklärung:\n"
            "-30 bis -60dBm – Sehr gute Signalstärke\n"
            "-60 bis -80dBm – ausreichende Signalstärke\n"
            "-80dBm – schwaches unzureichendes Signal\nIP-Addresse: *" +
            WiFi.localIP().toString() +
            "*\""
            "}"
            "},"
            "{"
            "\"type\": \"divider\""
            "},"
            "{"
            "\"type\": \"section\","
            "\"text\": {"
            "\"type\": \"mrkdwn\","
            "\"text\": \"Systemstatus:\nSystem läuft seit: *" +
            SysStartDate + " " + SysStartTime + "Uhr*\nLaufzeit: *" + String(LaufZeitDay) + " Tage " + String(LaufZeitHour) + "h " + String(LaufZeitMin) +
            "min*\""
            "}"
            "}"
            "]"
            "}",
        true);
  }
  else
  {
    WifiRebootConnected = true;
    PushSlackMsg(
        "{"
        "\"blocks\": ["
        "{"
        "\"type\": \"header\","
        "\"text\": {"
        "\"type\": \"plain_text\","
        "\"text\": \"System Neustart\","
        "\"emoji\": true"
        "}"
        "},"
        "{"
        "\"type\": \"section\","
        "\"text\": {"
        "\"type\": \"mrkdwn\","
        "\"text\": \"System wurde neu gestartet - Wlan verbunden!\""
        "}"
        "},"
        " {"
        "\"type\": \"divider\""
        "},"
        "{"
        "\"type\": \"section\","
        "\"text\": {"
        "\"type\": \"mrkdwn\","
        "\"text\": \"Neustartgrund:\nCore 0: " +
            print_reset_reason(rtc_get_reset_reason(0)) +
            "\nCore 1: " + print_reset_reason(rtc_get_reset_reason(1)) +
            "\""
            "}"
            "},"
            "{"
            "\"type\": \"divider\""
            "},"
            "{"
            "\"type\": \"section\","
            "\"text\": {"
            "\"type\": \"mrkdwn\","
            "\"text\": \"W-Lan:\nSignalstärke: *" +
            (String)WiFi.RSSI() +
            "dbm*\n-30 bis -60dBm – Sehr gute Signalstärke\n"
            "-60 bis -80dBm – ausreichende Signalstärke\n"
            "-80dBm – schwaches unzureichendes Signal\nIP-Addresse: *" +
            WiFi.localIP().toString() +
            "*\""
            "}"
            "}"
            "]"
            "}",
        true);
  }
  WifiNetworkAvaliable = true;
}

void WIFISetup()
{
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  // delete old config
  WiFi.disconnect(true);

  delay(1500);

  // Examples of different ways to register wifi events
  WiFi.onEvent(WiFiEvent);
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);
  WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.print(F("WiFi lost connection. Reason: "));
    Serial.println(info.disconnected.reason);
  },
                                       WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println();
  Serial.println(F("Wait for WiFi... "));
  delay(1000);
}

void CheckConnection()
{
  if (!WifiNetworkAvaliable)
  {
    WifiReconnectionAttempts++;
    if (WifiReconnectionAttempts % 3 == 0)
    {
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
    if (WifiReconnectionAttempts > 200)
    {
      ets_printf("reboot\n");
      esp_restart();
    }
  }
}

//**************************************** Send Test Msg ************************************************

void SendTestMsg()
{
  String Msg;
  //Test Msg
  Msg = "{"
        "\"blocks\": ["
        "{"
        "\"type\": \"header\","
        "\"text\": {"
        "\"type\": \"plain_text\","
        "\"text\": \"Testnachricht\","
        "\"emoji\": true"
        "}"
        "},"
        "{"
        "\"type\": \"divider\""
        "},"
        "{"
        "\"type\": \"section\","
        "\"text\": {"
        "\"type\": \"mrkdwn\","
        "\"text\": \"Datum: *" +
        formattedDate + "*\n Tagesleistung: *" +
        String(PTag) + "kWh*\nSpitzenleistung: *" + String(PMax) +
        "kW*\""
        "}"
        "},"
        "{"
        "\"type\": \"divider\""
        "},"
        "{"
        "\"type\": \"section\","
        "\"text\": {"
        "\"type\": \"mrkdwn\","
        "\"text\": \"W-Lan:\nSignalstärke: *" +
        (String)WiFi.RSSI() +
        "dbm*\n-30 bis -60dBm – Sehr gute Signalstärke\n"
        "-60 bis -80dBm – ausreichende Signalstärke\n"
        "-80dBm – schwaches unzureichendes Signal\n"
        "IP-Addresse: *" +
        WiFi.localIP().toString() +
        "*\""
        "}"
        "},"
        "{"
        "\"type\": \"divider\""
        "},"
        "{"
        "\"type\": \"section\","
        "\"text\": {"
        "\"type\": \"mrkdwn\","
        "\"text\": \"Systemstatus:\nSystem läuft seit: *" +
        SysStartDate + " " + SysStartTime + "Uhr*\nLaufzeit: *" + String(LaufZeitDay) + " Tage " + String(LaufZeitHour) + "h " + String(LaufZeitMin) +
        "min*\""
        "}"
        "}"
        "]"
        "}";

  PushSlackMsg(Msg, true);

  Serial.println(Msg);

  Msg = "";

  String htmlMsg = "<h1>Testmail</h1>"
                   "<p>Hallo " +
                   String(emailNameRecipient) +
                   ", heute den <b>" + formattedDate +
                   "</b> wurden <b>" + String(PTag) +
                   " kWh</b> Solarstrom produziert.<br>"
                   "Spitzenleistung <b>Pmax: " +
                   String(PMax) +
                   " kW</b></p><p>System läuft seit: <b>" +
                   SysStartDate + " " + SysStartTime +
                   "Uhr</b><br>Laufzeit: <b>" +
                   String(LaufZeitDay) + " Tage " + String(LaufZeitHour) + "h " + String(LaufZeitMin) +
                   "min</b><br>W-Lan:<br>IP-Addresse : <b>" +
                   WiFi.localIP().toString() + "</b><br>Signalstärke: <b>" + (String)WiFi.RSSI() +
                   "</b>dbm</p>"
                   "<p>Erklärung:"
                   "<br>-30 bis -60dBm – Sehr gute Signalstärke"
                   "<br>-60 bis -80dBm – ausreichende Signalstärke"
                   "<br>-80dBm – schwaches unzureichendes Signal</p>"
                   "<p>Halbstundenleistung:";

  for (byte i = 0; i < 48; i++)
  {
    htmlMsg = htmlMsg + "<br><nobr>" + PowerArrayToString(i) + "</nobr>";
  }
  htmlMsg = htmlMsg + "</p>";

  Serial.println(htmlMsg);

  EMailSender::EMailMessage message;
  message.subject = "Zusammenfassung Solaranlage";
  message.message = htmlMsg;

  EMailSender::Response resp = emailSend.send(emailRecipient, message);

  Serial.println(F("Sending status: "));
  if (resp.status)
  {
    Serial.println(resp.desc);
  }
  else
  {
    Serial.println("Failed");
  }
}

//**************************************** HTTPS ************************************************

bool SendSlackMsg(String URL, String Message)
{
  HTTPClient https;
  // Your Domain name with URL path or IP address with path
  https.begin(URL);

  // Specify content-type header
  https.addHeader("Content-Type", "application/json");
  // Data to send with HTTP POST
  String httpsRequestData = Message;
  //Serial.println(Message);
  // Send HTTP POST request
  int httpsResponseCode = https.POST(httpsRequestData);

  if (httpsRequestData.startsWith("HTTP/1.1"))
  {
    if (httpsRequestData.indexOf("200") < 0)
    {
      Serial.println(F("Got a non 200 status code from server."));
      return false;
    }
  }
  Serial.print(F("HTTP Response code: "));
  Serial.println(httpsResponseCode);
  //Free resources
  https.end();
  return true;
}

void PushSlackMsg(String Msg, bool Info)
{
  //  bool SlackMsgAvaliable[5];
  //  byte SlackMsgCounter;
  //  char *SlackDate[5];
  //  char *SlackTime[5];
  //  char *SlackMsg[5];
  //  bool SlackInfo[5];

  SlackMsgCounter++;
  SlackMsgCounter = SlackMsgCounter % 5;
  SlackMsg[SlackMsgCounter] = Msg;
  SlackInfo[SlackMsgCounter] = Info;
  SlackMsgAvaliable[SlackMsgCounter] = true;
}

void CheckToSendMsg()
{
  if ((!WifiNetworkAvaliable) || (!NTPOK))
  {
    return;
  }
  byte MsgCounter = SlackMsgCounter;
  for (int i = 0; i <= 6; i++)
  {
    MsgCounter++;
    MsgCounter = MsgCounter % 5;
    if (SlackMsgAvaliable[MsgCounter])
    {
      bool msgok = false;
      if (SlackInfo[MsgCounter] == true)
      {
        msgok = SendSlackMsg(SlackSysteminfos, SlackMsg[MsgCounter]);
      }
      else
      {
        msgok = SendSlackMsg(SlackStromerzeugung, SlackMsg[MsgCounter]);
      }
      if (msgok)
      {
        SlackMsgAvaliable[MsgCounter] = false;
      }
    }
  }
}

void CheckforReportTime(int Hour, int Minute)
{
  String Msg;
  String tmpExtra;
  byte TriggerHour = 21;
  byte TriggerMinute = 0;

  if (Minute % 30 == 0)
  {
    if (!TookMeasurement)
    {
      //Serial.println("Measuring Current");

      PowerArray[PowerArrayCounter] = PTag;
      PowerTimeArray[PowerArrayCounter] = digitstring(Hour) + ":" + digitstring(Minute);
      PowerArrayCounter++;
      TookMeasurement = true;
    }
  }
  else
  {
    TookMeasurement = false;
  }

  if ((Hour == TriggerHour) && (Minute == TriggerMinute) && (ReportSend == false))
  {
    if (LaufZeitDay < 1)
    {
      tmpExtra = Laufzeitkleiner24h;
    }
    else
    {
      tmpExtra = "Halbstundenleistung:\n";
      for (byte i = 0; i < 48; i++)
      {
        tmpExtra = tmpExtra + "\n" + PowerArrayToString(i);
      }
      tmpExtra = tmpExtra + "\n\n" + Laufzeithoeher24h;
    }
    ReportSend = true;

    Msg = "{"
          "\"blocks\": ["
          "{"
          "\"type\": \"header\","
          "\"text\": {"
          "\"type\": \"plain_text\","
          "\"text\": \"Täglicher Solaranlagenbericht\","
          "\"emoji\": true"
          "}"
          "},"
          "{"
          "\"type\": \"divider\""
          "},"
          "{"
          "\"type\": \"section\","
          "\"text\": {"
          "\"type\": \"mrkdwn\","
          "\"text\": \"Datum: *" +
          formattedDate + "*\n Tagesleistung: *" + String(PTag) + "kWh*\nSpitzenleistung: *" + String(PMax) +
          "kW*\""
          "}"
          "},"
          "{"
          "\"type\": \"divider\""
          "},"
          "{"
          "\"type\": \"section\","
          "\"text\": {"
          "\"type\": \"mrkdwn\","
          "\"text\": \"W-Lan:\nSignalstärke: *" +
          (String)WiFi.RSSI() +
          "dbm*\n"
          "-30 bis -60dBm – Sehr gute Signalstärke\n"
          "-60 bis -80dBm – ausreichende Signalstärke\n"
          "-80dBm – schwaches unzureichendes Signal\nIP-Addresse: *" +
          WiFi.localIP().toString() +
          "*\""
          "}"
          "},"
          "{"
          "\"type\": \"divider\""
          "},"
          "{"
          "\"type\": \"section\","
          "\"text\": {"
          "\"type\": \"mrkdwn\","
          "\"text\": \"Systemstatus:\nSystem läuft seit: *" +
          SysStartDate + " " + SysStartTime + "Uhr*\nLaufzeit: *" + String(LaufZeitDay) + " Tage " + String(LaufZeitHour) + "h " + String(LaufZeitMin) +
          "min*\""
          "}"
          "},"
          "{"
          "\"type\": \"divider\""
          "},"
          "{"
          "\"type\": \"section\","
          "\"text\": {"
          "\"type\": \"plain_text\","
          "\"text\": \"" +
          tmpExtra +
          "\""
          "}"
          "}"
          "]"
          "}";

    PushSlackMsg(Msg, false);

    MailPMax = PMax;
    MailPTag = PTag;
    PMax = 0;
    PTag = 0;
    PowerArrayCounter = 0;
    SendMail = true;
  }
  if ((Hour == TriggerHour) && (Minute > TriggerMinute))
  {
    ReportSend = false;
  }
}

String PowerArrayToString(byte ArrayNumber)
{
  String tmpString;
  byte PowerPosition;
  double Power = PowerArray[ArrayNumber];

  if (ArrayNumber > 0)
  {
    Power = Power - PowerArray[ArrayNumber - 1];
  }

  PowerPosition = (Power / 5.0);

  tmpString = "";
  for (byte i = 0; i <= 19; i++)
  {
    if (PowerPosition > i)
    {
      tmpString = tmpString + "█";
    }
    else
    {
      tmpString = tmpString + "░";
    }
  }
  tmpString = tmpString + " " + PowerTimeArray[ArrayNumber] + " - " + String(Power) + "kWh";
  return tmpString;
}

//**************************************** NTP ************************************************
void InitNTPTime()
{
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
}

void NTPTimeUpdate()
{
  unsigned long currentruntime = millis();
  if (currentruntime - previousLaufZeit >= 60000)
  {
    previousLaufZeit = currentruntime;
    LaufZeit++;
    LaufZeitDay = LaufZeit / 1440;
    LaufZeitHour = (LaufZeit / 60) - (LaufZeitDay * 24);
    LaufZeitMin = LaufZeit - (LaufZeitDay * 1440) - (LaufZeitHour * 60);
  }
  if (!WifiNetworkAvaliable)
  {
    return;
  }
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    FailedNTPAttempts++;
    NTPOK = false;
    if (FailedNTPAttempts > 10)
    {
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      NTPFail = true;
    }
    return;
  }
  if (NTPFail)
  {
    Serial.println(F("Time OK"));
  }
  NTPFail = false;
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

  CheckforReportTime(RunHour, RunMin);
  FailedNTPAttempts = 0;
  NTPOK = true;
}

String digitstring(int digit)
{
  String strdigit;
  if (digit < 10)
  {
    strdigit = "0";
  }
  strdigit = strdigit + String(digit);
  return strdigit;
}

//**************************************** SMTP MAIL ************************************************
void SendSMTPMail()
{
  String htmlMsg =
      "<h1>Täglicher Solaranlagenbericht</h1>"
      "<p>Hallo " +
      String(emailNameRecipient) + ", heute den <b>" +
      formattedDate + "</b> wurden <b>" +
      String(MailPTag) +
      " kWh</b> Solarstrom produziert.<br>"
      "Spitzenleistung <b>Pmax: " +
      String(MailPMax) +
      " kW</b></p>"
      "<p>System läuft seit: <b>" +
      SysStartDate + " " + SysStartTime +
      "Uhr</b><br>"
      "Laufzeit: <b>" +
      String(LaufZeitDay) + " Tage " + String(LaufZeitHour) + "h " + String(LaufZeitMin) +
      "min</b><br>"
      "W-Lan:<br>IP-Addresse : <b>" +
      WiFi.localIP().toString() + "</b><br>Signalstärke: <b>" + (String)WiFi.RSSI() +
      "</b>dbm</p>"
      "<p>Erklärung:"
      "<br>-30 bis -60dBm – Sehr gute Signalstärke"
      "<br>-60 bis -80dBm – ausreichende Signalstärke"
      "<br>-80dBm – schwaches unzureichendes Signal</p>";

  if (LaufZeitDay < 1)
  {
    htmlMsg = htmlMsg + "<p>" + Laufzeitkleiner24h + "</p>";
  }
  else
  {

    htmlMsg = htmlMsg + "<p>Halbstundenleistung:";
    for (byte i = 0; i < 48; i++)
    {
      htmlMsg = htmlMsg + "<br><nobr>" + PowerArrayToString(i) + "</nobr>";
    }
    htmlMsg = htmlMsg + "</p>";

    htmlMsg = htmlMsg + "<p>" + Laufzeithoeher24h + "</p>";
  }

  //Serial.println(htmlMsg);

  EMailSender::EMailMessage message;
  message.subject = "Zusammenfassung Solaranlage";
  message.message = htmlMsg;

  EMailSender::Response resp = emailSend.send(emailRecipient, message);

  Serial.println(F("Sending status: "));

  Serial.println(resp.status);
  Serial.println(resp.code);
  Serial.println(resp.desc);

  if (resp.status)
  {
    SendMail = false;
    FailedMailCounter = 0;
  }
  else
  {
    FailedMailCounter++;
    if (FailedMailCounter > 10)
    {
      SendMail = false;
    }
  }
}

void CheckToSendMail()
{
  if ((SendMail) && (WifiNetworkAvaliable) && (NTPOK))
  {
    SendSMTPMail();
  }
}

//**************************************** EMON LIB ************************************************
void InitCurrents()
{
  // Setup the ADC
  //analogSetAttenuation(ADC_11db);
  analogSetPinAttenuation(ADC_INPUT_A1, ADC_11db);
  analogSetPinAttenuation(ADC_INPUT_A2, ADC_11db);
  analogSetPinAttenuation(ADC_INPUT_A3, ADC_11db);

  analogReadResolution(ADC_BITS);

  pinMode(ADC_INPUT_A1, INPUT);
  pinMode(ADC_INPUT_A2, INPUT);
  pinMode(ADC_INPUT_A3, INPUT);

  emons1.current(ADC_INPUT_A1, 158.4); // Current: input pin, calibration.
  emons2.current(ADC_INPUT_A2, 160.0); // Current: input pin, calibration.
  emons3.current(ADC_INPUT_A3, 165.1); // Current: input pin, calibration.
}

double IrmsZero(double input)
{
  if (input < 1.0)
  {
    input = 0.0;
  }
  return input;
}

void CalculatePower()
{
  //1480
  Irms1 = IrmsZero(emons1.calcIrms(1480)); // Calculate Irms only
  Irms2 = IrmsZero(emons2.calcIrms(1480)); // Calculate Irms only
  Irms3 = IrmsZero(emons3.calcIrms(1480)); // Calculate Irms only

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
}

//**************************************** RESET REASON ************************************************
String print_reset_reason(RESET_REASON reason)
{
  switch (reason)
  {
  case 1:
    return "POWERON_RESET"; /**<1,  Vbat power on reset*/
  case 3:
    return "SW_RESET"; /**<3,  Software reset digital core*/
  case 4:
    return "OWDT_RESET"; /**<4,  Legacy watch dog reset digital core*/
  case 5:
    return "DEEPSLEEP_RESET"; /**<5,  Deep Sleep reset digital core*/
  case 6:
    return "SDIO_RESET"; /**<6,  Reset by SLC module, reset digital core*/
  case 7:
    return "TG0WDT_SYS_RESET"; /**<7,  Timer Group0 Watch dog reset digital core*/
  case 8:
    return "TG1WDT_SYS_RESET"; /**<8,  Timer Group1 Watch dog reset digital core*/
  case 9:
    return "RTCWDT_SYS_RESET"; /**<9,  RTC Watch dog Reset digital core*/
  case 10:
    return "INTRUSION_RESET"; /**<10, Instrusion tested to reset CPU*/
  case 11:
    return "TGWDT_CPU_RESET"; /**<11, Time Group reset CPU*/
  case 12:
    return "SW_CPU_RESET"; /**<12, Software reset CPU*/
  case 13:
    return "RTCWDT_CPU_RESET"; /**<13, RTC Watch dog Reset CPU*/
  case 14:
    return "EXT_CPU_RESET"; /**<14, for APP CPU, reseted by PRO CPU*/
  case 15:
    return "RTCWDT_BROWN_OUT_RESET"; /**<15, Reset when the vdd voltage is not stable*/
  case 16:
    return "RTCWDT_RTC_RESET"; /**<16, RTC Watch dog reset digital core and rtc module*/
  default:
    return "NO_MEAN";
  }
}
//**************************************** Button Inputs ************************************************

void InitSwitch()
{
  pinMode(SwitchLeft, INPUT);
  pinMode(SwitchRight, INPUT);

  pinMode(StatusLED, OUTPUT);
}

void CheckSwitchState()
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
    ClearDisplay = false;
    DisplayPauseCounter = 0;
    if (!ButtonPressed)
    {
      if (ButtonLeftState)
      {
        ActualDisplay++;
        ActualDisplay = ActualDisplay % 4;
      }
      if (ButtonRightState)
      {
        ActualDisplay--;
        ActualDisplay = ActualDisplay % 4;
      }
      ButtonPressed = true;
    }
    if ((currentMillis - SwitchpreviousMillis >= 5000) && (currentMillis - SwitchpreviousMillis < 10000))
    {
      TestMsgSend = true;
      SwitchpreviousMillis = currentMillis - 5000;
      ActualDisplay = 4;
      Serial.println("Test Start");
    }
  }
}

//**************************************** SETUP ************************************************
void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.print(F("CPU0 reset reason: "));
  Serial.println(print_reset_reason(rtc_get_reset_reason(0)));

  Serial.print(F("CPU1 reset reason: "));
  Serial.println(print_reset_reason(rtc_get_reset_reason(1)));

  delay(500);

  xTaskCreatePinnedToCore(
      TaskDisplay, "TaskDisplay", 2000 // Stack size
      ,
      NULL, 3 // Priority
      ,
      NULL, 0);
  delay(500);
  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
      TaskWifi, "TaskWifi" // A name just for humans
      ,
      5000 // This stack size can be checked & adjusted by reading the Stack Highwater
      ,
      NULL, 1 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
      ,
      NULL, 0);
  delay(500);
  xTaskCreate(
      TaskEmon, "TaskEmon", 1000 // Stack size
      ,
      NULL, 4 // Priority
      ,
      NULL);
  delay(500);
  xTaskCreate(
      TaskTime, "TaskTime", 2000 // Stack size
      ,
      NULL, 1 // Priority
      ,
      NULL);
  delay(500);
  xTaskCreate(
      TaskMsg, "TaskMsg", 6000 // Stack size
      ,
      NULL, 1 // Priority
      ,
      NULL);
  delay(500);
  xTaskCreate(
      TaskSwitch, "TaskSwitch", 1000 // Stack size
      ,
      NULL, 1 // Priority
      ,
      NULL);

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

//**************************************** MAIN LOOP ************************************************
void loop()
{
  vTaskDelay(1000);
}
