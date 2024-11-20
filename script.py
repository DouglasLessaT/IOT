import psycopg2

DB_CONFIG = {
    'dbname': 'iotdb',
    'user': 'root_api',
    'password': 'Douglas',
    'host': 'localhost',
    'port': '5432'
}

try:
    conn = psycopg2.connect(**DB_CONFIG)
    print("Conexão bem-sucedida!")
    conn.close()
except Exception as e:
    print(f"Erro na conexão: {e}")
