-- Criação do banco de dados IOT
CREATE DATABASE IF NOT EXISTS IOT;

-- Conectar-se ao banco de dados IOT
\c IOT;

-- Verifica se o usuário root_api já existe e cria se não existir
DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = 'root_api') THEN
        CREATE USER root_api WITH ENCRYPTED PASSWORD 'Douglas';
    END IF;
END $$;

-- Criação da tabela sensor_data
CREATE TABLE IF NOT EXISTS sensor_data (
    id SERIAL PRIMARY KEY,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ldr INT NOT NULL,
    relay BOOLEAN NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'active',
    agenda TIMESTAMP
);
