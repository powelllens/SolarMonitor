//All Settings for SolarMonitor are in this Headerfile to keep private Data private.

// Wifi Settings
const char* ssid = "SSID";
const char* password = "SSID-PW";

// Used for Slack Messages if Wifi Connection is established
// Domain Name with full URL Path for HTTPS POST Request
const char* SlackStromerzeugung = "https://hooks.slack.com/services/***/***/***";
const char* SlackSysteminfos = "https://hooks.slack.com/services/***/***/***";

// To send Email using Gmail use port 465 (SSL) and SMTP Server smtp.gmail.com
// YOU MUST ENABLE less secure app option https://myaccount.google.com/lesssecureapps?pli=1
#define AUTHOR_EMAIL    "sendermail[at]gmail.com"
#define AUTHOR_PASSWORD "sendermail-pw"
#define emailSubject    "Name Solaranlagen Info"

//Mail Empfänger
const char* emailRecipient = "RecieverMail@mail.de";
const char* emailNameRecipient = "Reciever Name";