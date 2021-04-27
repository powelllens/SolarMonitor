# SolarMonitor
Monitor for 100kW Solarpower - Measures 3Phase AC Current for healthchecks. Send daily report to Slack &amp; Mail.

# Used Board:
* Heltec Wifi Kit 32

# Sensor:
* Current Sensor SCT016S 150A/50mA

# Dependencies:
* NTPClient by Fabrice Weinberg for correct Date/Time
* WiFi by Arduino for general Ethernet Connection
* EMailSender by Renzo Mischianti for Mail notification
* EmonLib by OpenEnergyMonitor for Energy Monitoring
* Heltec ESP32 Dev-Boards by Heltec Automation for Display driver
* HTTPClient for Webhooks / Slack

