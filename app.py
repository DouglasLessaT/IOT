from flask import Flask, request, jsonify
import psycopg2
import paho.mqtt.client as mqtt
import json
from datetime import datetime
from dotenv import load_dotenv
import os

# Carrega variáveis de ambiente
load_dotenv()

app = Flask(__name__)

# Configurações do banco de dados PostgreSQL usando variáveis de ambiente
conn = psycopg2.connect(
    database=os.getenv("DB_NAME"),
    user=os.getenv("DB_USER"),
    password=os.getenv("DB_PASSWORD"),
    host=os.getenv("DB_HOST"),
    port=os.getenv("DB_PORT")
)
cursor = conn.cursor()

# Função para salvar dados no banco
def save_data(ldr, relay, status, id):
    query = "UPDATE sensor_data SET ldr = %s, relay = %s, status = %s WHERE id = %s"
    cursor.execute(query, (ldr, relay, status, id))
    conn.commit()

# Função MQTT para enviar comando à ESP32
def send_mqtt_message(topic, message):
    client.publish(topic, message)

# Funções MQTT
def on_connect(client, userdata, flags, rc):
    client.subscribe("esp32/response")

def on_message(client, userdata, msg):
    # Resposta da ESP32 indicando se o relé foi ligado corretamente
    data = json.loads(msg.payload)
    if data['status'] == 'success':
        # Atualiza o banco de dados com os novos valores
        cursor.execute("SELECT * FROM sensor_data WHERE status = 'active' ORDER BY timestamp DESC LIMIT 1")
        record = cursor.fetchone()
        if record:
            id = record[0]
            save_data(data['ldr'], data['relay'], 'active', id)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(os.getenv("BROKER_IP"), 1883, 60)
client.loop_start()

@app.route('/api/data', methods=['GET'])
def get_data():
    cursor.execute("SELECT * FROM sensor_data ORDER BY timestamp DESC LIMIT 10")
    records = cursor.fetchall()
    return jsonify(records)

@app.route('/api/control', methods=['POST'])
def control_relay():
    data = request.get_json()
    current_time = datetime.now()
    cursor.execute("SELECT * FROM sensor_data WHERE status = 'active' AND equipamento_ligado_em <= %s", (current_time,))
    records = cursor.fetchall()

    for record in records:
        # Envia um comando MQTT para ESP32 para ligar/desligar o relé
        message = json.dumps({"action": "toggle_relay", "time": str(current_time)})
        send_mqtt_message("esp32/command", message)

    return jsonify({"message": "Control signal sent"}), 200

if __name__ == "__main__":
    app.run(debug=True)
