#include <WiFi.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// --- SUA REDE WI-FI ---
const char* ssid     = "LightningSL"; 
const char* password = "LightningSL#2025";
const int SERVER_PORT = 8080;

WiFiServer server(SERVER_PORT);
WiFiClient remoteClient;

// --- PINOS DA TELA (Heltec Wireless Tracker) ---
#define TFT_MOSI 42
#define TFT_SCLK 41
#define TFT_CS   38
#define TFT_DC   40
#define TFT_RST  39
#define VEXT_PIN 3 // Pino mágico da Heltec que liga a energia da tela e do GPS
#define TFT_BL 21 // Pino comum de Backlight na família Heltec

// Instancia o driver da tela
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void setup() {
  Serial.begin(115200);
  
  // 1. Liga o LDO de Energia Principal da Placa (VEXT)
  pinMode(VEXT_PIN, OUTPUT);
  digitalWrite(VEXT_PIN, HIGH); 
  delay(100); // Precisa desse respiro para o hardware de energia estabilizar
  
  // 2. Liga a Luz de Fundo (Backlight) do TFT com brilho controlado (PWM)
  // analogWrite aceita valores de 0 (totalmente apagado) a 255 (brilho máximo)
  // Um valor entre 30 e 80 costuma ser excelente para ambientes internos
  analogWrite(TFT_BL, 10);

  // 3. Inicializa a Tela TFT
  tft.initR(INITR_MINI160x80); // Configuração exata da tela 0.96"
  tft.setRotation(3);          // Rotação horizontal
  tft.fillScreen(ST77XX_BLACK);
  
  // Mensagem Inicial
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(5, 5);
  tft.println("Iniciando...");
  tft.setCursor(5, 20);
  tft.println("Conectando Wi-Fi:");
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(5, 30);
  tft.println(ssid);
  
  // 4. Conecta no Wi-Fi
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWi-Fi Conectado!");
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());
  
  // 5. Atualiza Tela com IP conectado
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(5, 5);
  tft.println("Wi-Fi Conectado!");
  
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5, 25);
  tft.println("IP Local:");
  
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(5, 40);
  tft.println(WiFi.localIP().toString());
  
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(5, 65);
  tft.println("Aguardando Req...");
  
  server.begin();
}

void loop() {
  // 1. Atende USB Serial
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "GET DATA") {
      sendJson(Serial, "serial");
      updateScreenStatus("SERIAL (Cabo)");
    }
  }

  // 2. Atende Wi-Fi
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
      updateScreenStatus("WI-FI (Rede)");
    }
  }
}

// Função padronizada de envio JSON
template <typename T>
void sendJson(T &out, String src) {
  JsonDocument doc;
  doc["type"] = "data";
  doc["src"] = src; 
  
  JsonObject payload = doc["payload"].to<JsonObject>();
  // Gera valores aleatórios para teste
  payload["pm25"] = random(100, 500) / 10.0; 
  payload["pm10"] = random(200, 600) / 10.0;
  payload["co_ppm"] = random(0, 100) / 100.0;
  payload["lpg_ppm"] = random(200, 300);
  payload["temp_c"] = 25.5;
  payload["humid_p"] = 60.0;

  serializeJson(doc, out);
  out.println(); // Importante: Nova linha delimita o fim do JSON
}

// Função que pinta apenas a parte de baixo da tela
void updateScreenStatus(String modo) {
  // Apaga a mensagem anterior desenhando um quadrado preto por cima
  tft.fillRect(0, 55, 160, 25, ST77XX_BLACK); 
  
  tft.setCursor(5, 58);
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print("Req. Recebida:");
  
  tft.setCursor(5, 68);
  tft.setTextColor(ST77XX_ORANGE);
  tft.print(modo);
}