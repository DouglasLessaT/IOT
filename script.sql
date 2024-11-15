-- Verifica se o usuário root_api já existe e cria se não existir
DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = 'root_api') THEN
        CREATE USER root_api WITH ENCRYPTED PASSWORD 'Douglas';
    END IF;
END $$;

-- Criação da tabela sensor_data com a coluna status e equipamento_ligado_em
CREATE TABLE IF NOT EXISTS sensor_data (
    id SERIAL PRIMARY KEY,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ldr INT NOT NULL,
    relay BOOLEAN NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'active',  -- Coluna de status com valor padrão 'active'
    Agenda TIMESTAMP  -- Coluna para armazenar a data e hora para ligar o equipamento
);
