from flask import Flask, request, jsonify
import psycopg2
import paho.mqtt.client as mqtt
import json

app = Flask(__name__)

# Configurações do banco de dados
DB_HOST = "localhost"
DB_PORT = 5432
DB_USER = "your_user"
DB_PASSWORD = "your_password"
DB_NAME = "your_database"

# Configurações do broker MQTT
MQTT_BROKER = "mqtt-broker"
MQTT_PORT = 1883
MQTT_TOPIC = "esp32/command"

# Conexão com o banco de dados
conn = psycopg2.connect(
    database=DB_NAME, user=DB_USER, password=DB_PASSWORD, host=DB_HOST, port=DB_PORT
)
cursor = conn.cursor()

# Configuração do cliente MQTT
mqtt_client = mqtt.Client()
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)

@app.route('/api/send', methods=['POST'])
def send_command():
    data = request.get_json()
    id = data.get('id')

    # Obtém informações do banco de dados
    cursor.execute("SELECT * FROM schedule WHERE id = %s", (id,))
    record = cursor.fetchone()

    if not record:
        return jsonify({"error": "ID não encontrado"}), 404

    # Envia mensagem ao broker MQTT
    command = {"id": id, "action": "toggle_relay"}
    mqtt_client.publish(MQTT_TOPIC, json.dumps(command))

    return jsonify({"message": "Comando enviado ao broker"}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5001, debug=True)
