#include <WiFi.h>
#include <PubSubClient.h>
#include <IRremote.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  // Adicionado para suporte ao JSON
#include "secrets.h"      // Inclui o arquivo com as credenciais

#define LDR_PIN 34        // Pino analógico para o LDR
#define KY032_OUT_PIN 21  // Pino digital para o sinal de saída do KY-032
#define RELAY_PIN 33      // Pino digital para o relé
#define IR_LED_PIN 27     // Pino digital para o LED IR
#define KY032_LED_PIN 22  // LED indicador do estado do KY-032

#define LDR_THRESHOLD 200 // Limite de luminosidade para considerar "escuro"

IRsend irsend(IR_LED_PIN);  // Configura o pino do LED IR

WiFiClient espClient;
PubSubClient client(espClient);

bool relayState = false;    // Estado atual do relé
unsigned long relayTimer = 0; // Marca o tempo em que o relé foi ativado
unsigned long relayDuration = 10000; // Duração do relé ligado (10 segundos)

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  relayState = false;
  digitalWrite(RELAY_PIN, LOW); 
  irsend.begin(IR_LED_PIN);

  // Conexão Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao Wi-Fi...");
  }
  Serial.println("Conectado ao Wi-Fi");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;  // Declara o objeto http aqui

    http.begin("http://192.168.0.219:5000/esp32"); // URL da API

    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      String payload = http.getString();
      Serial.println("Resposta da API: " + payload);

      // Processa os dados recebidos
      DynamicJsonDocument doc(1024);  // Instancia o documento JSON
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.print("Erro ao analisar JSON: ");
        Serial.println(error.c_str());
        return;
      }

      // Verifica se contém chave "relay"
      if (doc.containsKey("relay")) {
        // Liga o relé
        digitalWrite(RELAY_PIN, HIGH);
        relayState = true;  // Atualiza o estado do relé
        relayTimer = millis();  // Armazena o tempo de ativação do relé

        // Envia o sinal IR se necessário
        if (doc.containsKey("irCode")) {
          long irCode = doc["irCode"];  // Lê o código IR da API
          irsend.sendNEC(irCode, 32);  // Envia o sinal IR
          Serial.println("Sinal IR enviado");
        }

        // Pisca o LED IR
        for (int i = 0; i < 3; i++) {
          irsend.sendNEC(0xF7C03F, 32);  // Envia o sinal IR de teste
          Serial.println("Sinal IR enviado");
          delay(500);  // Espera 500ms
          irsend.sendNEC(0x000000, 32);  // Desliga o LED IR
          delay(500);  // Espera 500ms
        }
        Serial.println("Relé ligado e LED IR piscando");
      } else {
        // Se não tiver a chave "relay", desliga o relé
        digitalWrite(RELAY_PIN, LOW);
        relayState = false;
        Serial.println("Relé desligado");
      }
    } else {
      Serial.println("Erro na requisição HTTP");
    }

    http.end();  // Finaliza a requisição HTTP
  } else {
    Serial.println("Erro na conexão Wi-Fi");
  }

  // Desliga o relé após 10 segundos
  if (relayState && (millis() - relayTimer >= relayDuration)) {
    digitalWrite(RELAY_PIN, LOW);  // Desliga o relé
    relayState = false;  // Atualiza o estado do relé
    Serial.println("Relé desligado após 10 segundos");
  }

  delay(1000); // Aguarda 1 segundo antes de repetir o loop
}
