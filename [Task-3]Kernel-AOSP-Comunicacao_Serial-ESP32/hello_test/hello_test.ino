void setup() {
  // Inicializa a Serial em 115200 bits por segundo
  Serial.begin(115200);
}

void loop() {
  // Envia uma mensagem a cada 1 segundo
  Serial.println("Android, voce esta me ouvindo?");
  delay(1000);
}
