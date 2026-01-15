#include <Arduino.h>
#include "SdsDustSensor.h"
#include "DHT.h"

/* ===================== DEFINIÇÕES ===================== */

#define MQ2_SENSOR 34
#define MQ7_SENSOR 35
#define DHT_SENSOR 4
#define DHTTYPE DHT11

#define SENSOR_UPDATE_INTERVAL 1000  // ms

/* ===================== OBJETOS ===================== */

DHT dht(DHT_SENSOR, DHTTYPE);
SdsDustSensor sds(Serial2);

/* ===================== VARIÁVEIS ===================== */

unsigned long lastSensorUpdate = 0;

int mq2Value = 0;
int mq7Value = 0;
float pm25Value = 0;
float pm10Value = 0;
float temperature = 0;
float humidity = 0;

/* ===================== SERIAL ===================== */

void processCommand(String cmd)
{
    cmd.trim();
    cmd.toUpperCase();

    if (cmd == "GET_MQ2")
        Serial.printf("RES GET_MQ2 %d\n", mq2Value);

    else if (cmd == "GET_MQ7")
        Serial.printf("RES GET_MQ7 %d\n", mq7Value);

    else if (cmd == "GET_PM25")
        Serial.printf("RES GET_PM25 %.1f\n", pm25Value);

    else if (cmd == "GET_PM10")
        Serial.printf("RES GET_PM10 %.1f\n", pm10Value);

    else if (cmd == "GET_TEMP")
        Serial.printf("RES GET_TEMP %.1f\n", temperature);

    else if (cmd == "GET_HUM")
        Serial.printf("RES GET_HUM %.1f\n", humidity);

    else if (cmd == "GET_ALL")
    {
        Serial.printf(
            "RES GET_ALL MQ2=%d MQ7=%d PM25=%.1f PM10=%.1f TEMP=%.1f HUM=%.1f\n",
            mq2Value, mq7Value, pm25Value, pm10Value, temperature, humidity
        );
    }
    else
        Serial.println("ERR UNKNOWN_COMMAND");
}

void handleSerial()
{
    static String command = "";

    while (Serial.available())
    {
        char c = Serial.read();
        command += c;

        if (c == '\n')
        {
            processCommand(command);
            command = "";
        }
    }
}

/* ===================== SENSORES ===================== */

void updateSensors()
{
    if (millis() - lastSensorUpdate < SENSOR_UPDATE_INTERVAL)
        return;

    lastSensorUpdate = millis();

    // MQ-2 e MQ-7
    mq2Value = map(analogRead(MQ2_SENSOR), 0, 4095, 0, 100);
    mq7Value = map(analogRead(MQ7_SENSOR), 0, 4095, 0, 100);

    // SDS011
    PmResult pm = sds.readPm();
    if (pm.isOk())
    {
        pm25Value = pm.pm25;
        pm10Value = pm.pm10;
    }

    // DHT11
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t))
    {
        humidity = h;
        temperature = t;
    }
}

/* ===================== SETUP ===================== */

void setup()
{
    Serial.begin(9600);

    sds.begin();
    dht.begin();

    analogSetAttenuation(ADC_11db);

    // Inicialização do SDS011
    sds.setActiveReportingMode();
    sds.setContinuousWorkingPeriod();

    Serial.println("DBG ENV_SENSOR Initialized");
}

/* ===================== LOOP ===================== */

void loop()
{
    handleSerial();
    updateSensors();
}