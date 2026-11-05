-- ============================================================================
--  Esquema do banco (MariaDB / MySQL)
--  Uso:  mariadb -u root -p < sql/schema.sql
--  (O programa também cria tudo isso sozinho ao salvar pela primeira vez.)
-- ============================================================================
CREATE DATABASE IF NOT EXISTS Sistema_de_Gerenciamento_de_consultas
  CHARACTER SET utf8mb4;
USE Sistema_de_Gerenciamento_de_consultas;

-- Cadastro mestre: um nome por ID de consulta/paciente.
CREATE TABLE IF NOT EXISTS pacientes (
  id   VARCHAR(20)  NOT NULL PRIMARY KEY,
  nome VARCHAR(100) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Pilha de consultas (LIFO). A ordem de inserção (id_registro) guarda do topo à base.
CREATE TABLE IF NOT EXISTS pilha_consultas (
  id_registro INT AUTO_INCREMENT PRIMARY KEY,
  paciente_id VARCHAR(20) NOT NULL,
  FOREIGN KEY (paciente_id) REFERENCES pacientes(id) ON UPDATE CASCADE
) ENGINE=InnoDB;

-- Fila de atendimento (FIFO). A ordem de id_registro é a ordem de chamada.
CREATE TABLE IF NOT EXISTS fila_atendimento (
  id_registro INT AUTO_INCREMENT PRIMARY KEY,
  paciente_id VARCHAR(20) NOT NULL,
  FOREIGN KEY (paciente_id) REFERENCES pacientes(id) ON UPDATE CASCADE
) ENGINE=InnoDB;
