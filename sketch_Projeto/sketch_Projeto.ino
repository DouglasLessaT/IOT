#include <WiFi.h>
#include <PubSubClient.h>
#include <IRremote.h>
#include "secrets.h"  // Inclui o arquivo com as credenciais

#define LDR_PIN 34        // Pino analógico para o LDR
#define PIR_PIN 21        // Pino digital para o sensor de presença (PIR)
#define RELAY_PIN 33      // Pino digital para o relé
#define IR_LED_PIN 13     // Pino digital para o LED IR

IRsend irsend(IR_LED_PIN);  // Configura o pino do LED IR

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  irsend.begin(IR_LED_PIN);  // Inicializa o envio de IR com o pino especificado

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao Wi-Fi...");
  }
  Serial.println("Conectado ao Wi-Fi");

  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(mqtt_callback);
  while (!client.connected()) {
    if (client.connect("ESP32Client", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("Conectado ao MQTT");
    } else {
      delay(5000);
    }
  }
}

void loop() {
  client.loop();
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // Se o comando for "toggle_relay", executa a ação
  if (message.indexOf("toggle_relay") >= 0) {
    digitalWrite(RELAY_PIN, HIGH);  // Liga o relé
    irsend.sendNEC(0x20DF10EF, 32); // Envia comando IR para o ar-condicionado
    delay(5000);  // Espera 5 segundos para verificar o status
    int ldrValue = analogRead(LDR_PIN);  // Verifica a luminosidade
    bool presenceDetected = digitalRead(PIR_PIN);  // Verifica o movimento

    // Envia resposta para a API com as informações
    String payload = "{\"status\": \"success\", \"ldr\": " + String(ldrValue) + ", \"relay\": " + String(digitalRead(RELAY_PIN)) + "}";
    client.publish("esp32/response", payload.c_str());
  }
}
