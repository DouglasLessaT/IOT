-- Criação do banco de dados IOT
CREATE DATABASE iotdb;
 CREATE USER root_api WITH ENCRYPTED PASSWORD 'Douglas';
-- Conectar-se ao banco de dados IOT
\c IOT;

-- Criação da tabela sensor_data
CREATE TABLE IF NOT EXISTS sensor_data (
    id SERIAL PRIMARY KEY,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ldr INT,
    relay BOOLEAN,
    status VARCHAR(50) DEFAULT 'Inative',
    agenda TIMESTAMP,
    tcc VARCHAR(255)
)

    REVOKE ALL PRIVILEGES ON TABLE sensor_data FROM root_api;
    GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE sensor_data TO root_api;
    GRANT USAGE ON SCHEMA public TO root_api;
    GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA public TO root_api;
    GRANT USAGE ON SCHEMA public TO root_api;
    GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA public TO root_api;
    GRANT USAGE, SELECT ON SEQUENCE sensor_data_id_seq TO root_api;