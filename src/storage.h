// Persistência das estruturas. Duas implementações: MariaDB/MySQL e CSV.
#ifndef STORAGE_H
#define STORAGE_H

#include <memory>
#include <string>
#include <vector>

#include "config.h"
#include "estruturas.h"

class Storage {
public:
    virtual ~Storage() = default;
    virtual std::string nome() const = 0;

    // pilha: do topo para a base. fila: do primeiro para o último.
    virtual bool salvar(const std::vector<Consulta>& pilha,
                        const std::vector<Consulta>& fila, std::string& erro) = 0;
    virtual bool carregar(std::vector<Consulta>& pilha,
                          std::vector<Consulta>& fila, std::string& erro) = 0;
};

std::unique_ptr<Storage> criarStorageCsv(const std::string& caminho);

// Retorna nullptr se o programa foi compilado sem suporte a MariaDB.
std::unique_ptr<Storage> criarStorageMariaDB(const Config& cfg);

bool temSuporteMariaDB();

#endif // STORAGE_H
