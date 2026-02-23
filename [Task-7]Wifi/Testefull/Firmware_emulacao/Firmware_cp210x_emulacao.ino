#include <WiFi.h>
#include <ArduinoJson.h>

// --- SUA REDE WI-FI ---
const char* ssid     = "LightningSL"; 
const char* password = "LightningSL#2025";
const int SERVER_PORT = 8080;

WiFiServer server(SERVER_PORT);
WiFiClient remoteClient;

void setup() {
  Serial.begin(115200);
  
  // Conecta no Wi-Fi
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWi-Fi Conectado!");
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());
  
  server.begin();
}

void loop() {
  // 1. Atende USB Serial (Cabo)
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "GET DATA") {
      sendJson(Serial, "serial");
    }
  }

  // 2. Atende Wi-Fi (Rede)
  if (server.hasClient()) {
    if (!remoteClient || !remoteClient.connected()) {
      remoteClient = server.available();
    }
  }

  if (remoteClient && remoteClient.connected() && remoteClient.available()) {
    String cmd = remoteClient.readStringUntil('\n');
    cmd.trim();
    if (cmd == "GET DATA") {
      sendJson(remoteClient, "wifi");
    }
  }
}

// Função padronizada de envio JSON usando Templates
template <typename T>
void sendJson(T &out, String src) {
  JsonDocument doc;
  doc["type"] = "data";
  doc["src"] = src; 
  
  JsonObject payload = doc["payload"].to<JsonObject>();
  
  // Gera valores aleatórios para o teste de bancada
  payload["pm25"] = random(100, 500) / 10.0; 
  payload["pm10"] = random(200, 600) / 10.0;
  payload["co_ppm"] = random(0, 100) / 100.0;
  payload["lpg_ppm"] = random(200, 300);
  payload["temp_c"] = 25.5;
  payload["humid_p"] = 60.0;

  serializeJson(doc, out);
  out.println(); // Importante: Nova linha delimita o fim do JSON
}