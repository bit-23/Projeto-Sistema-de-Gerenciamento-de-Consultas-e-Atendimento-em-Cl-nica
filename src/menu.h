#ifndef MENU_H
#define MENU_H

#include <memory>
#include <string>
#include <vector>

#include "config.h"
#include "estruturas.h"
#include "storage.h"

class ClinicaMenu {
public:
    ClinicaMenu(Config cfg, std::unique_ptr<Storage> storage);

    // Loop principal. Retorna o código de saída do programa.
    int executar();

    // Ações do menu principal
    void pushConsultas();
    void removeConsultas();
    void showConsultas();
    void loadData();
    void saveData();
    void managePilha();
    void manageFila();

    ClinicPilha pilhaDado;
    ClinicFila  filaDado;

private:
    int  selecionar(const std::string& titulo, const std::vector<std::string>& opcoes,
                    int& destaque, bool mostrarStatus);
    bool lerConsulta(Consulta& saida, bool paraPilha);
    void adicionarFila();
    void mostrarFila();
    bool confirmar(const std::string& pergunta);
    bool sairComSeguranca();
    void pausa();

    Config cfg_;
    std::unique_ptr<Storage> storage_;
    bool alterado_ = false;   // há mudanças não salvas?
};

#endif // MENU_H
