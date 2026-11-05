// Configuração do programa: valores padrão < arquivo clinica.conf < variáveis de ambiente.
#ifndef CONFIG_H
#define CONFIG_H

#include <string>

struct Config {
    std::string storage;     // "mariadb" ou "csv" (vazio = automático)
    std::string csvPath = "clinica_dados.csv";

    std::string dbHost = "127.0.0.1";
    std::string dbUser = "root";
    std::string dbSenha;
    std::string dbNome = "Sistema_de_Gerenciamento_de_consultas";
    unsigned    dbPorta = 3306;

    bool ascii = false;      // desenha o menu só com caracteres ASCII
};

// Carrega a configuração. Procura clinica.conf no diretório atual.
Config carregarConfig();

#endif // CONFIG_H
