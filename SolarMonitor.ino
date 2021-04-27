//Include Settings File
#include "SolarMonitorSettings.h"

//**************************************** DISPLAY ************************************************
#include "heltec.h"

// Change Display Interval
unsigned long DisplaypreviousMillis = 0;
unsigned long Displayinterval = 10000; // 10sec

// Display Refresh interval
unsigned long UpdateDisplaypreviousMillis = 0;
unsigned long UpdateDisplayinterval = 100; // 100ms

// Actual Display Number
byte ActualDisplay = 0;

void InitDisplay();
void ChangeDisplay();
void ShowWifiDisplay();
void ShowTimeDisplay();
void ShowSolarPower();
void ShowSolarAmp();

const unsigned char WIFI_bits[] PROGMEM = {
  0xF0, 0x03, 0x04, 0x08, 0xF2, 0x13, 0x09, 0x24, 0xE4, 0x09, 0x10, 0x02,
  0xC0, 0x00, 0xC0, 0x00,
};

//**************************************** WIFI ************************************************
#include <WiFi.h>

// Wifi Settings
//const char* ssid = "Defined in SolarMonitorSettings.h";
//const char* password = "Defined in SolarMonitorSettings.h";

//Check for Wifireconnection... Interval
unsigned long previousMillis = 0;
unsigned long interval = 1000; // 500ms
unsigned long previousMillisConnectionCheck = 0;
unsigned long ConnectionCheck = 3000; // 30sec

void WiFiEvent(WiFiEvent_t event);
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void WIFISetup();
void CheckConnection();

int WifiReconnectionAttempts = 0;
bool WifiRebootConnected = false;
bool WifiNetworkAvaliable = false;

//**************************************** HTTPS ************************************************
// HTTP POST infos für SLACK

#include <HTTPClient.h>

void SendSlackMsg(String URL, String Message);
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

const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 0;

String formattedTime = "No Time";
String formattedDate = "No Date";

String SysStartTime = "";
String SysStartDate = "";

void NTPTimeUpdate();
void printLocalTime();
void InitNTPTime();
String digitstring(int digit);
struct tm GetTime();

String Day;
String Month;

byte DayOld, MonthOld;

bool NTPOK = false;
byte FailedNTPAttempts = 0;
bool NTPFail = false;
unsigned long LaufZeit = 0;
unsigned long previousLaufZeit  = 0;
int LaufZeitDay = 0;
int LaufZeitHour = 0;
int LaufZeitMin = 0;

int RunHour;
int RunMin;

const String Laufzeitkleiner24h = "Achtung Systemlaufzeit kürzer als 24h - Messfehler möglich!";
const String Laufzeithoeher24h = "Alle Systeme laufen normal!";

//**************************************** WATCHDOG ************************************************
#include "esp_system.h"

const int wdtTimeout = 3000;  //time in ms to trigger the watchdog
hw_timer_t *timer = NULL;

void IRAM_ATTR resetModule();
void InitWatchdog();
void WatchdogReset();

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

bool SendMail = false;

byte FailedMailCounter = 0;


//**************************************** EMON LIB ************************************************
// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3

#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emons1;                  // Create an instance
EnergyMonitor emons2;                  // Create an instance
EnergyMonitor emons3;                  // Create an instance

double Irms1, Irms2, Irms3;
double P1, P2, P3;
double PGesamt;
double PTag;
double PMax;

unsigned long EmonpreviousMillis = 0;
unsigned long EmonPinterval = 1000; //1 sec

void InitCurrents();
void CalculatePower();

double PowerArray[48];
String PowerTimeArray[48];
byte PowerArrayCounter;

//**************************************** RESET REASON ************************************************
#include <rom/rtc.h>
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

String print_reset_reason(RESET_REASON reason);

//**************************************** Task Core 0 ************************************************

// define two tasks for Blink & AnalogRead
void TaskEthernet( void *pvParameters );
void TaskLocal( void *pvParameters );

//**************************************** SETUP ************************************************
void setup()
{
  Serial.begin(115200);
  delay(500);

  Serial.print(F("CPU0 reset reason: "));
  Serial.println(print_reset_reason(rtc_get_reset_reason(0)));

  Serial.print(F("CPU1 reset reason: "));
  Serial.println(print_reset_reason(rtc_get_reset_reason(1)));

  InitWatchdog();
  WatchdogReset();

  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
    TaskLocal
    ,  "TaskLocal"
    ,  10000  // Stack size
    ,  NULL
    ,  3  // Priority
    ,  NULL
    ,  0);

  WatchdogReset();
  delay(500);

  xTaskCreatePinnedToCore(
    TaskEthernet
    ,  "TaskEthernet"   // A name just for humans
    ,  50000  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL
    ,  1);

  WatchdogReset();
  delay(500);
  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

//**************************************** MAIN LOOP ************************************************
void loop() {
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskEthernet(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  // Intit Task
  WIFISetup();
  InitNTPTime();

  for (;;) // A Task shall never return or exit.
  {
    CheckConnection();
    NTPTimeUpdate();
    CheckToSendMsg();
    CheckToSendMail();
    vTaskDelay(10);  // one tick delay (15ms) in between reads for stability
  }
}

void TaskLocal(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  // Intit Task
  InitDisplay();

  InitWatchdog();
  WatchdogReset();

  InitCurrents();

  for (;;)
  {
    WatchdogReset();
    ChangeDisplay();
    CalculatePower();

    vTaskDelay(10);  // one tick delay (15ms) in between reads for stability
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
  unsigned long DisplaycurrentMillis = millis();

  // Display Change?
  if (DisplaycurrentMillis - DisplaypreviousMillis >= Displayinterval) {
    //Serial.println("Change Display");
    DisplaypreviousMillis = DisplaycurrentMillis;

    ActualDisplay ++;
    ActualDisplay = ActualDisplay % 4;
  }

  // Display Update?
  if (DisplaycurrentMillis - UpdateDisplaypreviousMillis >= UpdateDisplayinterval) {
    UpdateDisplaypreviousMillis = DisplaycurrentMillis;

    Heltec.display -> clear();
    Heltec.display -> drawString(86, 0, formattedTime);
    if (WifiNetworkAvaliable) {
      Heltec.display->drawXbm(0, 0, 14, 8, WIFI_bits);
    } else {
      Heltec.display -> drawString(6, 0, "-");
    }
    switch (ActualDisplay) {
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
      default: break;
    }
    Heltec.display -> display();
  }
}

void ShowWifiDisplay()
{
  Heltec.display -> drawString(16, 0, "W-LAN");
  if (WifiNetworkAvaliable) {
    Heltec.display -> drawString(0, 12, "Verbunden!");
    Heltec.display -> drawString(0, 24, "IP: " + WiFi.localIP().toString());
    Heltec.display -> drawString(0, 36, "RSSI: " + (String)WiFi.RSSI() + "dbm");
  } else {
    Heltec.display -> drawString(0, 12, "Keine Verbindung!");
    Heltec.display -> drawString(0, 24, "Letzter Versuch - " + String((millis() - previousMillis) / 1000) + "sek");
    Heltec.display -> drawString(0, 36, "Fehlversuche: " + String(WifiReconnectionAttempts));
  }
  Heltec.display -> drawString(0, 48, String(LaufZeitDay) + " Tage " + String(LaufZeitHour) + "h " + String(LaufZeitMin) + "min");
}

void ShowTimeDisplay()
{
  Heltec.display -> drawString(16, 0, "Datum/Zeit");
  Heltec.display -> drawString(0, 12, "Tag: " + Day);
  Heltec.display -> drawString(0, 24, "Monat: " + Month);
  Heltec.display -> drawString(0, 36, "Datum: " + formattedDate);
  Heltec.display -> drawString(0, 48, "Uhrzeit: " + formattedTime);
}

void ShowSolarAmp()
{
  Heltec.display -> drawString(16, 0, "Solar I & P");
  Heltec.display -> drawString(0, 14, "I1: " + String(Irms1) + "A");
  Heltec.display -> drawString(0, 28, "I2: " + String(Irms2) + "A");
  Heltec.display -> drawString(0, 42, "I3: " + String(Irms3) + "A");
  Heltec.display -> drawString(54, 14, "P1: " + String(P1) + "kW");
  Heltec.display -> drawString(54, 28, "P2: " + String(P2) + "kW");
  Heltec.display -> drawString(54, 42, "P3: " + String(P3) + "kW");
}

void ShowSolarPower()
{
  Heltec.display -> drawString(16, 0, "Solar-Power");

  Heltec.display -> drawString(0, 12, "Pges: " + String(PGesamt) + "kW");
  Heltec.display -> drawString(0, 24, "Pmax: " + String(PMax) + "kW");
  Heltec.display -> drawString(0, 36, "Tag Leistung:");
  Heltec.display -> drawString(0, 48, String(PTag) + "kWh");
}

//**************************************** WIFI ************************************************
void WiFiEvent(WiFiEvent_t event)
{
  Serial.printf("[WiFi-event] event: %d\n", event);

  switch (event) {
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
    default: break;
  }
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  WifiReconnectionAttempts = 0;
  if (WifiRebootConnected) {
    PushSlackMsg("{"
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
                 "\"text\": \"W-Lan:\nSignalstärke: *" + (String)WiFi.RSSI() + "dbm*\nErklärung:\n-30 bis -60dBm – Sehr gute Signalstärke\n-60 bis -80dBm – ausreichende Signalstärke\n-80dBm – schwaches unzureichendes Signal\nIP-Addresse: *" + WiFi.localIP().toString() + "*\""
                 "}"
                 "},"
                 "{"
                 "\"type\": \"divider\""
                 "},"
                 "{"
                 "\"type\": \"section\","
                 "\"text\": {"
                 "\"type\": \"mrkdwn\","
                 "\"text\": \"Systemstatus:\nSystem läuft seit: *" + SysStartDate + " " + SysStartTime + "Uhr*\nLaufzeit: *" + String(LaufZeitDay) + " Tage " + String(LaufZeitHour) + "h " + String(LaufZeitMin) + "min*\""
                 "}"
                 "}"
                 "]"
                 "}"
                 , true);
  } else {
    WifiRebootConnected = true;
    PushSlackMsg("{"
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
                 "\"text\": \"Neustartgrund:\nCore 0: " + print_reset_reason(rtc_get_reset_reason(0)) + "\nCore 1: " + print_reset_reason(rtc_get_reset_reason(1)) + "\""
                 "}"
                 "},"
                 "{"
                 "\"type\": \"divider\""
                 "},"
                 "{"
                 "\"type\": \"section\","
                 "\"text\": {"
                 "\"type\": \"mrkdwn\","
                 "\"text\": \"W-Lan:\nSignalstärke: *" + (String)WiFi.RSSI() + "dbm*\n-30 bis -60dBm – Sehr gute Signalstärke\n-60 bis -80dBm – ausreichende Signalstärke\n-80dBm – schwaches unzureichendes Signal\nIP-Addresse: *" + WiFi.localIP().toString() + "*\""
                 "}"
                 "}"
                 "]"
                 "}"
                 , true);
  }
  WifiNetworkAvaliable = true;
}


void WIFISetup()
{
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  // delete old config
  WiFi.disconnect(true);

  delay(1000);

  // Examples of different ways to register wifi events
  WiFi.onEvent(WiFiEvent);
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);
  WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.print(F("WiFi lost connection. Reason: "));
    Serial.println(info.disconnected.reason);
  }, WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println();
  Serial.println(F("Wait for WiFi... "));
  delay(500);
}

void CheckConnection()
{
  unsigned long currentruntime = millis();
  if ((currentruntime - previousMillisConnectionCheck >= ConnectionCheck) && !WifiNetworkAvaliable) {
    previousMillisConnectionCheck = currentruntime;
    WifiReconnectionAttempts ++;
    if (WifiReconnectionAttempts % 10 == 0)
    {
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
    if (WifiReconnectionAttempts > 300)
    {
      ets_printf("reboot\n");
      esp_restart();
    }
  }
}

//**************************************** HTTPS ************************************************

void SendSlackMsg(String URL, String Message)
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

  if (httpsRequestData.startsWith("HTTP/1.1")) {
    if (httpsRequestData.indexOf("200") < 0) {
      Serial.println(F("Got a non 200 status code from server."));
    }
  }
  Serial.print(F("HTTP Response code: "));
  Serial.println(httpsResponseCode);
  //Free resources
  https.end();
}

void PushSlackMsg(String Msg, bool Info)
{
  //  bool SlackMsgAvaliable[5];
  //  byte SlackMsgCounter;
  //  char *SlackDate[5];
  //  char *SlackTime[5];
  //  char *SlackMsg[5];
  //  bool SlackInfo[5];

  SlackMsgCounter ++;
  SlackMsgCounter = SlackMsgCounter % 5;
  SlackMsg[SlackMsgCounter] = Msg;
  SlackInfo[SlackMsgCounter] = Info;
  SlackMsgAvaliable[SlackMsgCounter] = true;
}

void CheckToSendMsg()
{
  if ((!WifiNetworkAvaliable) || (!NTPOK)) {
    return;
  }
  byte MsgCounter = SlackMsgCounter;
  for (int i = 0; i <= 6; i++) {
    MsgCounter ++;
    MsgCounter = MsgCounter % 5;
    if (SlackMsgAvaliable[MsgCounter]) {
      if (SlackInfo[MsgCounter] == true) {
        SendSlackMsg(SlackSysteminfos, SlackMsg[MsgCounter]);
      } else {
        SendSlackMsg(SlackStromerzeugung, SlackMsg[MsgCounter]);
      }
      SlackMsgAvaliable[MsgCounter] = false;
    }
  }
}

void CheckforReportTime(int Hour, int Minute)
{
  String Msg;
  String tmpExtra;
  byte TriggerHour = 21;
  byte TriggerMinute = 0;

  if (Minute % 30 == 0) {
    if (!TookMeasurement) {
      //Serial.println("Measuring Current");

      PowerArray[PowerArrayCounter] = PTag;
      PowerTimeArray[PowerArrayCounter] = digitstring(Hour) + ":" + digitstring(Minute);
      PowerArrayCounter ++;
      TookMeasurement = true;
    }
  } else {
    TookMeasurement = false;
  }

  if ((Hour == TriggerHour ) && (Minute == TriggerMinute) && (ReportSend == false)) {
    if (LaufZeitDay < 1) {
      tmpExtra = Laufzeitkleiner24h;
    } else {
      tmpExtra = "Halbstundenleistung:\n";
      for (byte i = 0; i < 48; i++) {
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
          "\"text\": \"Datum: *" + formattedDate + "*\n Tagesleistung: *" + String(PTag) + "kWh*\nSpitzenleistung: *" + String(PMax) + "kW*\""
          "}"
          "},"
          "{"
          "\"type\": \"divider\""
          "},"
          "{"
          "\"type\": \"section\","
          "\"text\": {"
          "\"type\": \"mrkdwn\","
          "\"text\": \"W-Lan:\nSignalstärke: *" + (String)WiFi.RSSI() + "dbm*\n-30 bis -60dBm – Sehr gute Signalstärke\n-60 bis -80dBm – ausreichende Signalstärke\n-80dBm – schwaches unzureichendes Signal\nIP-Addresse: *" + WiFi.localIP().toString() + "*\""
          "}"
          "},"
          "{"
          "\"type\": \"divider\""
          "},"
          "{"
          "\"type\": \"section\","
          "\"text\": {"
          "\"type\": \"mrkdwn\","
          "\"text\": \"Systemstatus:\nSystem läuft seit: *" + SysStartDate + " " + SysStartTime + "Uhr*\nLaufzeit: *" + String(LaufZeitDay) + " Tage " + String(LaufZeitHour) + "h " + String(LaufZeitMin) + "min*\""
          "}"
          "},"
          "{"
          "\"type\": \"divider\""
          "},"
          "{"
          "\"type\": \"section\","
          "\"text\": {"
          "\"type\": \"plain_text\","
          "\"text\": \"" + tmpExtra + "\""
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
  if ((Hour == TriggerHour ) && (Minute > TriggerMinute)) {
    ReportSend = false;
  }
}

String PowerArrayToString(byte ArrayNumber)
{
  String tmpString;
  byte PowerPosition;
  double Power = PowerArray[ArrayNumber];

  if (ArrayNumber > 0) {
    Power = Power  - PowerArray[ArrayNumber - 1];
  }

  PowerPosition = (Power / 5.0);

  tmpString = "";
  for (byte i = 0; i <= 19; i++) {
    if (PowerPosition > i) {
      tmpString = tmpString + "█";
    } else {
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
  if (!WifiNetworkAvaliable) {
    return;
  }
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    FailedNTPAttempts ++;
    NTPOK = false;
    if (FailedNTPAttempts > 10) {
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      NTPFail = true;
    }
    return;
  }
  if (NTPFail) {
    Serial.println(F("Time OK"));
  }
  NTPFail = false;
  formattedTime = digitstring(timeinfo.tm_hour) + ":" + digitstring(timeinfo.tm_min) + ":" + digitstring(timeinfo.tm_sec);
  formattedDate = digitstring(timeinfo.tm_mday) + "." + digitstring(1 + timeinfo.tm_mon) + "." + String(1900 + timeinfo.tm_year);

  RunHour = timeinfo.tm_hour;
  RunMin = timeinfo.tm_min;

  if (SysStartTime == "") {
    SysStartTime = formattedTime;
    SysStartDate = formattedDate;
  }

  if (timeinfo.tm_wday != DayOld) {
    DayOld = timeinfo.tm_wday;
    switch (timeinfo.tm_wday) {
      case 1:
        Day = "Montag";
        break;
      case 2:
        Day = "Dienstag";
        break;
      case 3:
        Day = "Mitwoch";
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
      case 7:
        Day = "Sonntag";
        break;
      default: break;
    }
  }
  if (timeinfo.tm_mon != MonthOld) {
    MonthOld = timeinfo.tm_mon;
    switch (timeinfo.tm_mon) {
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
      default: break;
    }
  }
  unsigned long currentruntime = millis();
  if (currentruntime - previousLaufZeit >= 60000) {
    previousLaufZeit = currentruntime;
    LaufZeit ++;
    LaufZeitDay = LaufZeit / 1440;
    LaufZeitHour = (LaufZeit / 60) - (LaufZeitDay * 24);
    LaufZeitMin = LaufZeit - (LaufZeitDay * 1440) - (LaufZeitHour * 60) ;
  }
  CheckforReportTime(RunHour, RunMin);
  FailedNTPAttempts = 0;
  NTPOK = true;
}

String digitstring(int digit)
{
  String strdigit;
  if (digit < 10) {
    strdigit = "0";
  }
  strdigit = strdigit + String(digit);
  return strdigit;
}

//**************************************** WATCHDOG ************************************************
void IRAM_ATTR resetModule()
{
  ets_printf("reboot\n");
  esp_restart();
}

void InitWatchdog()
{
  timer = timerBegin(0, 80, true);                  //timer 0, div 80
  timerAttachInterrupt(timer, &resetModule, true);  //attach callback
  timerAlarmWrite(timer, wdtTimeout * 1000, false); //set time in us
  timerWrite(timer, 0); //reset timer (feed watchdog)
  timerAlarmEnable(timer);                          //enable interrupt
}

void WatchdogReset()
{
  timerWrite(timer, 0); //reset timer (feed watchdog)
}

//**************************************** SMTP MAIL ************************************************
void SendSMTPMail()
{
  String htmlMsg = "<h1>Täglicher Solaranlagenbericht</h1>"
                   "<p>Hallo " + String(emailNameRecipient) + ", heute den <b>" + formattedDate  + "</b> wurden <b>" + String(MailPTag) + " kWh</b> Solarstrom produziert.<br>"
                   "Spitzenleistung <b>Pmax: " + String(MailPMax) + " kW</b></p>"
                   "<p>System läuft seit: <b>" + SysStartDate + " " + SysStartTime + "Uhr</b><br>"
                   "Laufzeit: <b>" + String(LaufZeitDay) + " Tage " + String(LaufZeitHour) + "h " + String(LaufZeitMin) + "min</b><br>"
                   "W-Lan:<br>IP-Addresse : <b>"  + WiFi.localIP().toString() + "</b><br>Signalstärke: <b>" + (String)WiFi.RSSI() + "</b>dbm</p>"
                   "<p>Erklärung:"
                   "<br>-30 bis -60dBm – Sehr gute Signalstärke"
                   "<br>-60 bis -80dBm – ausreichende Signalstärke"
                   "<br>-80dBm – schwaches unzureichendes Signal</p>";

  if (LaufZeitDay < 1) {
    htmlMsg = htmlMsg + "<p>" + Laufzeitkleiner24h + "</p>";
  } else {

    htmlMsg = htmlMsg + "<p>Halbstundenleistung:";
    for (byte i = 0; i < 48; i++) {
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

  if (resp.status) {
    SendMail = false;
    FailedMailCounter = 0;
  } else {
    delay(1000);
    FailedMailCounter ++;
    if (FailedMailCounter > 10) {
      SendMail = false;
    }
  }
}

void CheckToSendMail()
{
  if ((SendMail) && (WifiNetworkAvaliable) && (NTPOK)) {
    SendSMTPMail();
  }
}

//**************************************** EMON LIB ************************************************
void InitCurrents()
{
  emons1.current(36, 111.1);             // Current: input pin, calibration.
  emons2.current(37, 111.1);             // Current: input pin, calibration.
  emons3.current(38, 111.1);             // Current: input pin, calibration.
}

void CalculatePower()
{
  unsigned long currentMillis = millis();

  Irms1 = emons1.calcIrms(1480);  // Calculate Irms only
  Irms2 = emons2.calcIrms(1480);  // Calculate Irms only
  Irms3 = emons3.calcIrms(1480);  // Calculate Irms only

  P1 = (Irms1 * 230.0) / 1000.0; // Apparent power
  P2 = (Irms2 * 230.0) / 1000.0; // Apparent power
  P3 = (Irms3 * 230.0) / 1000.0; // Apparent power

  PGesamt = P1 + P2 + P3;
  if (PMax < PGesamt) {
    PMax = PGesamt;
  }

  if (currentMillis - EmonpreviousMillis >= EmonPinterval) {
    EmonpreviousMillis = currentMillis;
    PTag = PTag + (PGesamt / 3600);
  }
}

//**************************************** RESET REASON ************************************************
String print_reset_reason(RESET_REASON reason)
{
  switch (reason)
  {
    case 1 : return "POWERON_RESET";         /**<1,  Vbat power on reset*/
    case 3 : return "SW_RESET";              /**<3,  Software reset digital core*/
    case 4 : return "OWDT_RESET";            /**<4,  Legacy watch dog reset digital core*/
    case 5 : return "DEEPSLEEP_RESET";       /**<5,  Deep Sleep reset digital core*/
    case 6 : return "SDIO_RESET";            /**<6,  Reset by SLC module, reset digital core*/
    case 7 : return "TG0WDT_SYS_RESET";      /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8 : return "TG1WDT_SYS_RESET";      /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9 : return "RTCWDT_SYS_RESET";      /**<9,  RTC Watch dog Reset digital core*/
    case 10 : return "INTRUSION_RESET";      /**<10, Instrusion tested to reset CPU*/
    case 11 : return "TGWDT_CPU_RESET";      /**<11, Time Group reset CPU*/
    case 12 : return "SW_CPU_RESET";         /**<12, Software reset CPU*/
    case 13 : return "RTCWDT_CPU_RESET";     /**<13, RTC Watch dog Reset CPU*/
    case 14 : return "EXT_CPU_RESET";        /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : return "RTCWDT_BROWN_OUT_RESET"; /**<15, Reset when the vdd voltage is not stable*/
    case 16 : return "RTCWDT_RTC_RESET";     /**<16, RTC Watch dog reset digital core and rtc module*/
    default : return "NO_MEAN";
  }
}
