#include <WiFi.h>
#include <PubSubClient.h>
#include <IRremote.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  // Adicionado para suporte ao JSON
#include "secrets.h"      // Inclui o arquivo com as credenciais

#define LDR_PIN 34        // Pino analógico para o LDR
#define PIR_PIN 21        // Pino digital para o sensor de presença (PIR)
#define RELAY_PIN 33      // Pino digital para o relé
#define IR_LED_PIN 13     // Pino digital para o LED IR

#define RELAY_TIMEOUT_MS 5000  // Tempo em milissegundos para o relé desarmar

IRsend irsend(IR_LED_PIN);  // Configura o pino do LED IR

WiFiClient espClient;
PubSubClient client(espClient);

bool relayState = false;    // Estado atual do relé
unsigned long relayTimer = 0; // Marca o tempo em que o relé foi ativado

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
    HTTPClient http;
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

      if (doc.containsKey("relay")) {
        bool relay = doc["relay"];
        if (relay && !relayState) {
          digitalWrite(RELAY_PIN, HIGH);  // Liga o relé (lógica invertida)
          irsend.sendNEC(0xF7C03F, 32); // Envia sinal IR
          Serial.println("Relé ligado e sinal IR enviado");
          relayState = true;
          relayTimer = millis(); // Inicia o temporizador
        }
      }
    } else {
      Serial.println("Erro ao obter dados: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("Erro na conexão Wi-Fi");
  }

  // Verifica se o tempo de ativação do relé excedeu o limite
  if (relayState && (millis() - relayTimer >= RELAY_TIMEOUT_MS)) {
    digitalWrite(RELAY_PIN, HIGH); // Desliga o relé (lógica invertida)
    relayState = false;
    Serial.println("Relé desarmado automaticamente após o timeout");
  }

  delay(1000);
}
