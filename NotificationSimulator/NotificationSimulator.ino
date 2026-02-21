#include <Arduino.h>
#include <ArduinoJson.h>

// ==========================================
// ESTADO
// ==========================================

String inputBuffer = "";

// Calibração simulada
float calib_sds = 1.0;
float calib_mq2 = 9.8;
float calib_mq7 = 15.2;
float calib_temp = -1.0;
float calib_hum = 2.0;


// ==========================================
// CICLOS DE TESTE AUTOMÁTICOS
// ==========================================

// Cada ciclo = 10 min
const unsigned long CYCLE_TIME = 600000; // 10 min

// Total = 40 min
const unsigned long TOTAL_TIME = CYCLE_TIME * 4;

unsigned long testStart = 0;


// ==========================================
// SETUP
// ==========================================

void setup() {

  Serial.begin(115200);

  inputBuffer.reserve(200);

  randomSeed(analogRead(0));

  testStart = millis();
}


// ==========================================
// LOOP
// ==========================================

void loop() {

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
}


// ==========================================
// PROCESSADOR
// ==========================================

void processCommand(String cmd) {

  cmd.trim();
  cmd.toUpperCase();


  if (cmd == "GET DATA" || cmd == "GET DATA ALL") {
    sendSensorData("ALL");
  }

  else if (cmd == "GET DATA SDS011") {
    sendSensorData("SDS011");
  }

  else if (cmd == "GET DATA MQ2") {
    sendSensorData("MQ2");
  }

  else if (cmd == "GET DATA MQ7") {
    sendSensorData("MQ7");
  }

  else if (cmd == "GET DATA DHT") {
    sendSensorData("DHT");
  }

  else if (cmd == "GET SETTINGS") {
    sendSettings();
  }

  else if (cmd.startsWith("SET CALIB ")) {
    handleCalibration(cmd);
  }

  else if (cmd == "GET STATUS") {
    sendStatus();
  }

  else if (cmd == "GET METADATA") {
    sendMetadata();
  }
}


// ==========================================
// SIMULAÇÃO BASE
// ==========================================

float simulateValue(float base, float variance, float speed) {

  return base +
         (sin(millis() / speed) * variance) +
         ((random(-10, 10) / 10.0));
}


// ==========================================
// CONTROLE DE CICLO
// ==========================================

int getCurrentCycle() {

  unsigned long elapsed = millis() - testStart;

  unsigned long pos = elapsed % TOTAL_TIME;

  return pos / CYCLE_TIME; // 0..3
}


// ==========================================
// ENVIO DE DADOS
// ==========================================

void sendSensorData(String target) {

  JsonDocument doc;

  doc["type"] = "data";
  doc["src"] = "serial";

  if (target != "ALL") {
    target.toLowerCase();
    doc["sensor"] = target;
  }

  JsonObject payload = doc["payload"].to<JsonObject>();


  // ==============================
  // CICLO ATUAL
  // ==============================

  int cycle = getCurrentCycle();


  float pm25;
  float co_ppm;


  switch (cycle) {

    // --------------------------------
    // CICLO 0: NORMAL (SEM ALERTA)
    // --------------------------------
    case 0:

      pm25 = simulateValue(10, 2, 4000);
      co_ppm = simulateValue(1.0, 0.3, 8000);

      break;


    // --------------------------------
    // CICLO 1: SÓ CO
    // --------------------------------
    case 1:

      pm25 = simulateValue(10, 2, 4000);
      co_ppm = random(150, 250) / 10.0; // 15~25

      break;


    // --------------------------------
    // CICLO 2: SÓ PM2.5
    // --------------------------------
    case 2:

      pm25 = simulateValue(45, 5, 3000); // >35 médio
      co_ppm = simulateValue(1.0, 0.3, 8000);

      break;


    // --------------------------------
    // CICLO 3: AMBOS
    // --------------------------------
    default:

      pm25 = simulateValue(45, 5, 3000);
      co_ppm = random(150, 250) / 10.0;
  }


  // ==============================
  // OUTROS
  // ==============================

  float temp = simulateValue(26.0, 2.0, 5000.0);
  float hum  = simulateValue(60.0, 10.0, 8000.0);


  // ==============================
  // PAYLOAD
  // ==============================

  if (target == "ALL" || target == "sds011") {

    payload["pm25"] = round(pm25 * 10) / 10.0;
    payload["pm10"] = round((pm25 * 1.5) * 10) / 10.0;
  }


  if (target == "ALL" || target == "mq2") {

    payload["lpg_ppm"] = (int)simulateValue(200, 50, 4000);

    if (target == "mq2")
      payload["raw_val"] = random(1400, 1500);
  }


  if (target == "ALL" || target == "mq7") {

    payload["co_ppm"] = round(co_ppm * 100) / 100.0;

    if (target == "mq7")
      payload["raw_val"] = random(700, 900);
  }


  if (target == "ALL" || target == "dht") {

    payload["temp_c"]  = round(temp * 10) / 10.0;
    payload["humid_p"] = round(hum * 10) / 10.0;
  }


  serializeJson(doc, Serial);
  Serial.println();
}


// ==========================================
// SETTINGS
// ==========================================

void sendSettings() {

  JsonDocument doc;

  doc["type"] = "settings";
  doc["device_id"] = "AIR_STATION_SIMULATOR";

  JsonObject wifi = doc["wifi"].to<JsonObject>();
  wifi["ssid"] = "AndroidAP_Sim";
  wifi["ip"] = "0.0.0.0";


  JsonObject calib = doc["calib"].to<JsonObject>();

  calib["sds_factor"] = calib_sds;
  calib["mq2_ro"]     = calib_mq2;
  calib["mq7_ro"]     = calib_mq7;
  calib["temp_offset"] = calib_temp;
  calib["hum_offset"]  = calib_hum;


  serializeJson(doc, Serial);
  Serial.println();
}


// ==========================================
// CALIB
// ==========================================

void handleCalibration(String cmd) {

  int a = cmd.indexOf(' ');
  int b = cmd.indexOf(' ', a + 1);
  int c = cmd.indexOf(' ', b + 1);

  if (b == -1 || c == -1) return;


  String target = cmd.substring(b + 1, c);
  String valStr = cmd.substring(c + 1);

  float val = valStr.toFloat();


  if (target == "SDS")  calib_sds  = val;
  else if (target == "MQ2")  calib_mq2  = val;
  else if (target == "MQ7")  calib_mq7  = val;
  else if (target == "TEMP") calib_temp = val;
  else if (target == "HUM")  calib_hum  = val;


  JsonDocument doc;

  doc["type"] = "ack";
  doc["cmd"]  = "set_calib";

  target.toLowerCase();

  doc["target"]  = target;
  doc["new_val"] = val;
  doc["status"]  = "saved";


  serializeJson(doc, Serial);
  Serial.println();
}


// ==========================================
// STATUS
// ==========================================

void sendStatus() {

  JsonDocument doc;

  doc["type"] = "status";

  doc["uptime_sec"] = millis() / 1000;
  doc["wifi_status"] = "disconnected";


  JsonObject sensors = doc["sensors"].to<JsonObject>();

  sensors["sds011"] = "ok";
  sensors["mq2"] = (millis() < 10000) ? "warming_up" : "ok";
  sensors["mq7"] = (millis() < 10000) ? "warming_up" : "ok";
  sensors["dht11"] = "ok";


  serializeJson(doc, Serial);
  Serial.println();
}


// ==========================================
// METADATA
// ==========================================

void sendMetadata() {

  JsonDocument doc;

  doc["type"] = "metadata";


  JsonArray sensors = doc["sensors"].to<JsonArray>();


  JsonObject s1 = sensors.add<JsonObject>();
  s1["id"] = "pm25";
  s1["name"] = "PM 2.5";
  s1["unit"] = "ug/m3";


  JsonObject s2 = sensors.add<JsonObject>();
  s2["id"] = "pm10";
  s2["name"] = "PM 10";
  s2["unit"] = "ug/m3";


  JsonObject s3 = sensors.add<JsonObject>();
  s3["id"] = "lpg_ppm";
  s3["name"] = "GLP";
  s3["unit"] = "ppm";


  JsonObject s4 = sensors.add<JsonObject>();
  s4["id"] = "co_ppm";
  s4["name"] = "Monoxido Carbono";
  s4["unit"] = "ppm";


  JsonObject s5 = sensors.add<JsonObject>();
  s5["id"] = "temp_c";
  s5["name"] = "Temperatura";
  s5["unit"] = "C";


  JsonObject s6 = sensors.add<JsonObject>();
  s6["id"] = "humid_p";
  s6["name"] = "Umidade";
  s6["unit"] = "%";


  serializeJson(doc, Serial);
  Serial.println();
}