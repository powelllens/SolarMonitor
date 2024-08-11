#include "influxdb.h"

influxdbConClass::influxdbConClass() {};

void influxdbConClass::setup(String INFLUXDB_URL, String INFLUXDB_ORG, String INFLUXDB_BUCKET, String INFLUXDB_TOKEN)
{
    client.setConnectionParams(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, ServerCert);

    // Enable messages batching and retry buffer
    client.setWriteOptions(WriteOptions().writePrecision(WRITE_PRECISION).batchSize(MAX_BATCH_SIZE).bufferSize(WRITE_BUFFER_SIZE));

    // Set insecure connection to skip server certificate validation
    // client.setInsecure();
    /*
    temp_sensor.dac_offset = TSENS_DAC_L2; // TSENS_DAC_L2 is default; L4(-40°C ~ 20°C), L2(-10°C ~ 80°C), L1(20°C ~ 100°C), L0(50°C ~ 125°C)
    temp_sensor_set_config(temp_sensor);
    temp_sensor_start();
    */
}

void influxdbConClass::loop()
{
    if (millis() - lastDataTime > updateinterval || initalUpload)
    {

        lastDataTime = millis();
        if (WiFi.status() == WL_CONNECTED && emonmon != nullptr && timesynced)
        {

            // Check server connection
            if (client.validateConnection())
            {
                DEBUG_LINE_("Connected to InfluxDB: ");
                DEBUG_LINE(client.getServerUrl());
                connectionfailedcounter = 0;
                // Check whether buffer in not empty
                if (!client.isBufferEmpty())
                {
                    // Write all remaining points to db
                    client.flushBuffer();
                }
                initalUpload = false;
            }
            else
            {
                DEBUG_LINE_("InfluxDB connection failed: ");
                DEBUG_LINE(client.getLastErrorMessage());
                delay(5000);
                ++connectionfailedcounter;
                if (connectionfailedcounter > 3)
                {
                    ESP.restart();
                }
            }

            /*
                        float temp = 0;
                        temp_sensor_read_celsius(&temp);
            */
            // Set identical time for the whole network scan
            if ((initalUpload && initaldata) || (!initaldata && !initalUpload))
            {
                initaldata = false;
                time_t tnow = time(nullptr);

                Point record("measurement");

                record.addTag("location", "adersbach");

                record.addField("p1", emonmon->getP1());
                record.addField("p2", emonmon->getP2());
                record.addField("p3", emonmon->getP3());

                record.addField("Irms1", emonmon->getIrms1());
                record.addField("Irms2", emonmon->getIrms2());
                record.addField("Irms3", emonmon->getIrms3());

                record.addField("PGesamt", emonmon->getPGesamt());
                record.addField("PTag", emonmon->getPTag());
                record.addField("PMax", emonmon->getPMax());

                record.addField("RSSI", WiFi.RSSI());
                /*
                            record.addField("Temp", temp);
                */
                record.addField("Uptime", timelib->getUptime());

                record.setTime(tnow);

                client.writePoint(record);
            }
        }
    }
}

void influxdbConClass::setUpdateInterval(uint16_t interval)
{
    updateinterval = interval * 1000;
}

void influxdbConClass::setupEmon(emon *_emon)
{
    emonmon = _emon;
}

void influxdbConClass::setupTime(timeNTP *_timelib)
{
    timelib = _timelib;
}
