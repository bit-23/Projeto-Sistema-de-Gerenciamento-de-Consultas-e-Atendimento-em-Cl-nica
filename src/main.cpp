// Sistema de Gerenciamento de Consultas e Atendimento em Clínica
// Multiplataforma: Linux, macOS e Windows.
#include <cstring>
#include <iostream>

#include "config.h"
#include "menu.h"
#include "storage.h"
#include "terminal.h"

#ifndef CLINICA_VERSAO
#define CLINICA_VERSAO "2.0.0"
#endif

static void ajuda() {
    std::cout <<
        "Uso: clinica [opções]\n\n"
        "  --csv        usa arquivo CSV em vez do MariaDB\n"
        "  --mariadb    usa o MariaDB (exige build com suporte)\n"
        "  --ascii      menu só com caracteres ASCII (consoles antigos)\n"
        "  --versao     mostra a versão\n"
        "  --ajuda      mostra esta ajuda\n\n"
        "Configuração: arquivo clinica.conf ou variáveis CLINICA_* (veja o README.md).\n";
}

int main(int argc, char** argv) {
    Config cfg = carregarConfig();

    for (int i = 1; i < argc; ++i) {
        if      (!std::strcmp(argv[i], "--csv"))     cfg.storage = "csv";
        else if (!std::strcmp(argv[i], "--mariadb")) cfg.storage = "mariadb";
        else if (!std::strcmp(argv[i], "--ascii"))   cfg.ascii = true;
        else if (!std::strcmp(argv[i], "--versao") || !std::strcmp(argv[i], "--version")) {
            std::cout << "clinica " CLINICA_VERSAO "\n"; return 0;
        }
        else if (!std::strcmp(argv[i], "--ajuda") || !std::strcmp(argv[i], "--help") || !std::strcmp(argv[i], "-h")) {
            ajuda(); return 0;
        }
        else { std::cerr << "Opção desconhecida: " << argv[i] << "\n\n"; ajuda(); return 1; }
    }

    term::iniciar();
    term::setModoAscii(cfg.ascii);

    // Escolha do armazenamento: MariaDB quando disponível, senão CSV.
    std::unique_ptr<Storage> storage;
    std::string aviso;
    bool quer = cfg.storage.empty() || cfg.storage == "mariadb";
    if (quer && temSuporteMariaDB()) {
        storage = criarStorageMariaDB(cfg);
    } else {
        if (cfg.storage == "mariadb")
            aviso = "Este executável foi compilado sem suporte a MariaDB. Usando CSV.";
        storage = criarStorageCsv(cfg.csvPath);
    }

    if (!aviso.empty()) {
        std::cout << term::AMARELO << aviso << term::RESET << "\n";
        std::cout << "Pressione qualquer tecla para continuar...";
        std::cout.flush();
        term::lerTecla();
    }

    ClinicaMenu menu(cfg, std::move(storage));
    return menu.executar();
}
