from flask import Flask, request, jsonify
from flask_cors import CORS  # Importe o CORS
import psycopg2
from datetime import datetime

# Configuração do banco de dados
DB_CONFIG = {
    'dbname': 'iotdb',
    'user': 'root_api',
    'password': 'Douglas',
    'host': 'localhost',
    'port': '5432'
}

# Conectar ao banco de dados
def get_db_connection():
    conn = psycopg2.connect(**DB_CONFIG)
    return conn

app = Flask(__name__)

# Permitir CORS para todas as origens
CORS(app)

# Rota para fazer POST e inserir novo TCC
@app.route('/agenda', methods=['POST'])
def add_agenda():
    data = request.json
    tcc = data.get('tcc')
    datetime_field = data.get('datetime')  # Recebe data e hora
    relay = data.get('relay', False)

    if not tcc or not datetime_field:
        return jsonify({'error': 'Os campos "tcc" e "datetime" são obrigatórios!'}), 400

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute(
            """
            INSERT INTO sensor_data (relay, status, agenda)
            VALUES (%s, 'Inative', %s) RETURNING id;
            """,
            (relay, datetime_field)
        )
        inserted_id = cursor.fetchone()[0]
        conn.commit()
        cursor.close()
        conn.close()

        return jsonify({'message': 'TCC adicionado com sucesso!', 'id': inserted_id}), 201

    except Exception as e:
        return jsonify({'error': str(e)}), 500

# Rota para fazer GET e obter dados para ESP32
@app.route('/dados', methods=['GET'])
def get_data():
    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        # Seleciona o próximo TCC com status "Inative" e agenda para agora ou anterior
        cursor.execute(
            """
            SELECT id, agenda, relay FROM sensor_data
            WHERE status = 'Inative' AND agenda <= NOW()
            ORDER BY agenda ASC LIMIT 1;
            """
        )
        data = cursor.fetchone()

        if data:
            id, datetime_field, relay = data
            # Atualiza o status para "Active"
            cursor.execute(
                "UPDATE sensor_data SET status = 'Active' WHERE id = %s;",
                (id,)
            )
            conn.commit()
            response = {'id': id, 'agenda': datetime_field, 'relay': relay}
        else:
            response = {'message': 'Nenhuma agenda disponível.'}

        cursor.close()
        conn.close()
        return jsonify(response), 200

    except Exception as e:
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
