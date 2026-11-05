#include "config.h"

#include <cstdlib>
#include <fstream>

namespace {

std::string trim(const std::string& s) {
    const char* esp = " \t\r\n";
    auto a = s.find_first_not_of(esp);
    if (a == std::string::npos) return "";
    auto b = s.find_last_not_of(esp);
    return s.substr(a, b - a + 1);
}

bool verdadeiro(const std::string& v) {
    return v == "1" || v == "true" || v == "sim" || v == "yes" || v == "on";
}

void aplicar(Config& c, const std::string& chave, const std::string& valor) {
    if      (chave == "storage")  c.storage = valor;
    else if (chave == "csv_path") c.csvPath = valor;
    else if (chave == "db_host")  c.dbHost = valor;
    else if (chave == "db_user")  c.dbUser = valor;
    else if (chave == "db_pass")  c.dbSenha = valor;
    else if (chave == "db_name")  c.dbNome = valor;
    else if (chave == "db_port")  { int p = std::atoi(valor.c_str()); if (p > 0 && p < 65536) c.dbPorta = (unsigned)p; }
    else if (chave == "ascii")    c.ascii = verdadeiro(valor);
}

void lerArquivo(Config& c, const std::string& caminho) {
    std::ifstream f(caminho);
    std::string linha;
    while (std::getline(f, linha)) {
        linha = trim(linha);
        if (linha.empty() || linha[0] == '#' || linha[0] == ';') continue;
        auto eq = linha.find('=');
        if (eq == std::string::npos) continue;
        aplicar(c, trim(linha.substr(0, eq)), trim(linha.substr(eq + 1)));
    }
}

void lerEnv(Config& c, const char* var, const char* chave) {
    if (const char* v = std::getenv(var)) aplicar(c, chave, v);
}

} // namespace

Config carregarConfig() {
    Config c;
    lerArquivo(c, "clinica.conf");
    lerEnv(c, "CLINICA_STORAGE", "storage");
    lerEnv(c, "CLINICA_CSV",     "csv_path");
    lerEnv(c, "CLINICA_DB_HOST", "db_host");
    lerEnv(c, "CLINICA_DB_USER", "db_user");
    lerEnv(c, "CLINICA_DB_PASS", "db_pass");
    lerEnv(c, "CLINICA_DB_NAME", "db_name");
    lerEnv(c, "CLINICA_DB_PORT", "db_port");
    lerEnv(c, "CLINICA_ASCII",   "ascii");
    return c;
}
