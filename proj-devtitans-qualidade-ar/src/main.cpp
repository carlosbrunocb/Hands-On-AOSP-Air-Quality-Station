#include <Arduino.h>
#include <ArduinoJson.h>
#include "SdsDustSensor.h"
#include "DHT.h"

/* ===================== DEFINIÇÕES ===================== */

#define MQ2_SENSOR 34
#define MQ7_SENSOR 35
#define DHT_SENSOR 4
#define DHTTYPE DHT11

#define SENSOR_UPDATE_INTERVAL 1000

/* ===================== OBJETOS ===================== */

DHT dht(DHT_SENSOR, DHTTYPE);
SdsDustSensor sds(Serial2);

/* ===================== VARIÁVEIS ===================== */

String inputBuffer = "";

unsigned long lastUpdate = 0;

int mq2_raw = 0;
int mq7_raw = 0;
float pm25 = 0;
float pm10 = 0;
float temperature = 0;
float humidity = 0;

// Fatores de calibração
float calib_sds = 1.0;
float calib_mq2 = 1.0;
float calib_mq7 = 1.0;
float calib_temp = 0.0;
float calib_hum = 0.0;

/* ===================== LEITURA DOS SENSORES ===================== */

void updateSensors() {

  // MQ
  mq2_raw = analogRead(MQ2_SENSOR);
  mq7_raw = analogRead(MQ7_SENSOR);

  // SDS011
  PmResult pm = sds.readPm();
  if (pm.isOk()) {
    pm25 = pm.pm25 * calib_sds;
    pm10 = pm.pm10 * calib_sds;
  }

  // DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    humidity = h + calib_hum;
    temperature = t + calib_temp;
  }
}

/* ===================== JSON RESPONSES ===================== */

void sendSensorData(String target) {

  JsonDocument doc;

  doc["type"] = "data";
  doc["src"] = "serial";

  JsonObject payload = doc["payload"].to<JsonObject>();

  if (target == "ALL" || target == "SDS011") {
    payload["pm25"] = round(pm25 * 10) / 10.0;
    payload["pm10"] = round(pm10 * 10) / 10.0;
  }

  if (target == "ALL" || target == "MQ2") {
    payload["mq2_raw"] = mq2_raw;
  }

  if (target == "ALL" || target == "MQ7") {
    payload["mq7_raw"] = mq7_raw;
  }

  if (target == "ALL" || target == "DHT") {
    payload["temp_c"] = round(temperature * 10) / 10.0;
    payload["humid_p"] = round(humidity * 10) / 10.0;
  }

  serializeJson(doc, Serial);
  Serial.println();
}

void sendSettings() {
  JsonDocument doc;
  doc["type"] = "settings";
  doc["device_id"] = "AIR_STATION_REAL";

  JsonObject calib = doc["calib"].to<JsonObject>();
  calib["sds_factor"] = calib_sds;
  calib["mq2_factor"] = calib_mq2;
  calib["mq7_factor"] = calib_mq7;
  calib["temp_offset"] = calib_temp;
  calib["hum_offset"] = calib_hum;

  serializeJson(doc, Serial);
  Serial.println();
}

void handleCalibration(String cmd) {

  // Esperado: SET CALIB SDS 1.1
  int firstSpace = cmd.indexOf(' ');
  int secondSpace = cmd.indexOf(' ', firstSpace + 1);
  int thirdSpace = cmd.indexOf(' ', secondSpace + 1);

  if (secondSpace == -1 || thirdSpace == -1)
    return;

  String target = cmd.substring(secondSpace + 1, thirdSpace);
  String valStr = cmd.substring(thirdSpace + 1);

  float val = valStr.toFloat();

  if (target == "SDS") calib_sds = val;
  else if (target == "MQ2") calib_mq2 = val;
  else if (target == "MQ7") calib_mq7 = val;
  else if (target == "TEMP") calib_temp = val;
  else if (target == "HUM") calib_hum = val;
  else return;

  JsonDocument doc;
  doc["type"] = "ack";
  doc["cmd"] = "set_calib";

  target.toLowerCase();
  doc["target"] = target;
  doc["new_val"] = val;
  doc["status"] = "saved";

  serializeJson(doc, Serial);
  Serial.println();
}


void sendStatus() {
  JsonDocument doc;
  doc["type"] = "status";
  doc["uptime_sec"] = millis() / 1000;

  JsonObject sensors = doc["sensors"].to<JsonObject>();
  sensors["sds011"] = "ok";
  sensors["mq2"] = "ok";
  sensors["mq7"] = "ok";
  sensors["dht11"] = "ok";

  serializeJson(doc, Serial);
  Serial.println();
}

void sendMetadata() {
  JsonDocument doc;
  doc["type"] = "metadata";

  JsonArray sensors = doc["sensors"].to<JsonArray>();

  JsonObject s1 = sensors.add<JsonObject>();
  s1["id"] = "pm25"; s1["unit"] = "ug/m3";

  JsonObject s2 = sensors.add<JsonObject>();
  s2["id"] = "pm10"; s2["unit"] = "ug/m3";

  JsonObject s3 = sensors.add<JsonObject>();
  s3["id"] = "mq2_raw"; s3["unit"] = "adc";

  JsonObject s4 = sensors.add<JsonObject>();
  s4["id"] = "mq7_raw"; s4["unit"] = "adc";

  JsonObject s5 = sensors.add<JsonObject>();
  s5["id"] = "temp_c"; s5["unit"] = "C";

  JsonObject s6 = sensors.add<JsonObject>();
  s6["id"] = "humid_p"; s6["unit"] = "%";

  serializeJson(doc, Serial);
  Serial.println();
}

/* ===================== PROCESSADOR ===================== */

void processCommand(String cmd) {

  cmd.toUpperCase();

  if (cmd == "GET DATA" || cmd == "GET DATA ALL")
    sendSensorData("ALL");

  else if (cmd == "GET DATA SDS011")
    sendSensorData("SDS011");

  else if (cmd == "GET DATA MQ2")
    sendSensorData("MQ2");

  else if (cmd == "GET DATA MQ7")
    sendSensorData("MQ7");

  else if (cmd == "GET DATA DHT")
    sendSensorData("DHT");

  else if (cmd == "GET SETTINGS")
    sendSettings();

  else if (cmd.startsWith("SET CALIB "))
    handleCalibration(cmd);

  else if (cmd == "GET STATUS")
    sendStatus();

  else if (cmd == "GET METADATA")
    sendMetadata();
}

/* ===================== SETUP ===================== */

void setup() {
  Serial.begin(115200);
  inputBuffer.reserve(200);

  dht.begin();
  sds.begin();
  sds.setActiveReportingMode();
  sds.setContinuousWorkingPeriod();

  analogSetAttenuation(ADC_11db);

  Serial.println("{\"type\":\"boot\",\"device\":\"AIR_STATION_REAL\"}");
}

/* ===================== LOOP ===================== */

void loop() {

  // ===== SERIAL HANDLER =====
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n') {
      inputBuffer.trim();
      processCommand(inputBuffer);
      inputBuffer = "";
    } else {
      inputBuffer += c;
    }
  }

  // ===== ATUALIZA SENSORES =====
  if (millis() - lastUpdate > SENSOR_UPDATE_INTERVAL) {
    updateSensors();
    lastUpdate = millis();
  }
}