from flask import Flask, jsonify
import paho.mqtt.client as mqtt
import json

app = Flask(__name__)

# Configurações do broker MQTT
MQTT_BROKER = "mqtt-broker"
MQTT_PORT = 1883
MQTT_TOPIC = "esp32/response"

# Variáveis para armazenar dados recebidos
last_message = None

# Função chamada ao receber uma mensagem do broker MQTT
def on_message(client, userdata, msg):
    global last_message
    payload = json.loads(msg.payload.decode())
    last_message = payload
    print(f"Mensagem recebida: {payload}")

# Configuração do cliente MQTT
mqtt_client = mqtt.Client()
mqtt_client.on_message = on_message
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
mqtt_client.subscribe(MQTT_TOPIC)
mqtt_client.loop_start()

@app.route('/api/status', methods=['GET'])
def get_status():
    if not last_message:
        return jsonify({"message": "Nenhuma mensagem recebida ainda"}), 404

    return jsonify(last_message), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5002, debug=True)
