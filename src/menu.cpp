#include "menu.h"

#include <iostream>

#include "terminal.h"

using namespace term;

namespace {

constexpr int LARGURA = 39;   // largura da moldura, igual à versão original

// Conta caracteres UTF-8 (não bytes).
std::size_t tamanhoUtf8(const std::string& s) {
    std::size_t n = 0;
    for (unsigned char c : s) if ((c & 0xC0) != 0x80) ++n;
    return n;
}

std::string trim(const std::string& s) {
    const char* esp = " \t\r\n";
    auto a = s.find_first_not_of(esp);
    if (a == std::string::npos) return "";
    return s.substr(a, s.find_last_not_of(esp) - a + 1);
}

std::string centralizar(const std::string& t) {
    std::size_t n = tamanhoUtf8(t);
    std::size_t esq = n < (std::size_t)LARGURA ? (LARGURA - n) / 2 : 0;
    std::size_t dir = n < (std::size_t)LARGURA ? LARGURA - n - esq : 0;
    return std::string(esq, ' ') + t + std::string(dir, ' ');
}

void moldura() {
    std::cout << CIANO << NEGRITO << std::string(LARGURA, '=') << RESET << "\n";
}

} // namespace

ClinicaMenu::ClinicaMenu(Config cfg, std::unique_ptr<Storage> storage)
    : cfg_(std::move(cfg)), storage_(std::move(storage)) {}

// ---------------------------------------------------------------------------
// Menu interativo com setas (estilo archinstall)
// ---------------------------------------------------------------------------
int ClinicaMenu::selecionar(const std::string& titulo, const std::vector<std::string>& opcoes,
                            int& destaque, bool mostrarStatus) {
    const int n = (int)opcoes.size();
    const bool ascii = modoAscii();

    while (true) {
        limparTela();
        moldura();
        std::cout << CIANO << NEGRITO << centralizar(titulo) << RESET << "\n";
        moldura();
        std::cout << "Use as " << VERDE << (ascii ? "setas (^/v)" : "setas (▲/▼)") << RESET
                  << " para navegar e " << VERDE << "Enter" << RESET << " para confirmar:\n\n";

        for (int i = 0; i < n; i++) {
            if (i == destaque)
                std::cout << REVERSO << VERDE << NEGRITO << (ascii ? "  >  " : "  ➔  ")
                          << opcoes[i] << "  " << RESET << "\n";
            else
                std::cout << "     " << opcoes[i] << "\n";
        }
        std::cout << "\n";
        moldura();
        if (mostrarStatus) {
            std::cout << "Pilha: " << VERDE << pilhaDado.size() << RESET
                      << "  |  Fila: " << VERDE << filaDado.size() << RESET
                      << "  |  Dados: " << CIANO << (storage_ ? storage_->nome() : "-") << RESET;
            if (alterado_) std::cout << "  " << AMARELO << "[não salvo]" << RESET;
            std::cout << "\n";
        }
        std::cout.flush();

        Entrada e = lerTecla();
        switch (e.tecla) {
            case Tecla::Cima:   destaque = (destaque - 1 + n) % n; break;
            case Tecla::Baixo:  destaque = (destaque + 1) % n; break;
            case Tecla::Enter:  return destaque;
            case Tecla::Esc:    return -1;
            case Tecla::Numero:
                if (e.numero >= 1 && e.numero <= n) { destaque = e.numero - 1; return destaque; }
                break;
            default: break;
        }
    }
}

void ClinicaMenu::pausa() {
    std::cout << "\nPressione " << VERDE << "qualquer tecla" << RESET
              << " para voltar ao menu principal...";
    std::cout.flush();
    lerTecla();
}

bool ClinicaMenu::confirmar(const std::string& pergunta) {
    std::cout << pergunta << " (" << VERDE << "s" << RESET << "/" << VERMELHO << "n" << RESET << "): ";
    std::cout.flush();
    while (true) {
        Entrada e = lerTecla();
        if (e.ch == 's' || e.ch == 'S') { std::cout << "sim\n"; return true; }
        if (e.ch == 'n' || e.ch == 'N' || e.tecla == Tecla::Esc) { std::cout << "não\n"; return false; }
    }
}

// ---------------------------------------------------------------------------
// Loop principal
// ---------------------------------------------------------------------------
int ClinicaMenu::executar() {
    int atual = 0;
    const std::string banco = storage_ ? storage_->nome() : "?";

    while (true) {
        std::vector<std::string> opcoes = {
            "1. Adicionar Consulta (Pilha)",
            "2. Remover Consulta (Pilha)",
            "3. Mostrar Consultas",
            "4. Carregar Dados do " + banco,
            "5. Salvar Dados no " + banco,
            "6. Gerenciar Pilha",
            "7. Gerenciar Fila",
            "8. Sair do Programa",
        };
        int escolha = selecionar("SISTEMA DE GESTÃO CLÍNICA", opcoes, atual, true);
        if (escolha < 0) continue;   // Esc no menu principal: ignora

        if (escolha == 7) {
            if (sairComSeguranca()) {
                std::cout << "\nSaindo do programa de forma segura...\n";
                return 0;
            }
            continue;
        }

        // Os submenus têm o próprio loop e não precisam de "pausa".
        if (escolha == 5) { managePilha(); continue; }
        if (escolha == 6) { manageFila();  continue; }

        limparTela();
        std::cout << CIANO << NEGRITO << "[Executando ação...]" << RESET << "\n\n";
        switch (escolha) {
            case 0: pushConsultas();   break;
            case 1: removeConsultas(); break;
            case 2: showConsultas();   break;
            case 3: loadData();        break;
            case 4: saveData();        break;
        }
        pausa();
    }
}

bool ClinicaMenu::sairComSeguranca() {
    if (!alterado_) return true;
    limparTela();
    std::cout << AMARELO << "Há alterações que ainda não foram salvas." << RESET << "\n";
    if (confirmar("Salvar antes de sair?")) {
        saveData();
        if (alterado_) {                       // falhou ao salvar
            return confirmar("\nSair mesmo assim e perder as alterações?");
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// Entrada de dados
// ---------------------------------------------------------------------------

// Lê ID e nome (o nome pode ter espaços). Retorna false se o usuário cancelar
// (ID em branco) ou se os dados forem inválidos.
bool ClinicaMenu::lerConsulta(Consulta& c, bool paraPilha) {
    std::string id, nome;
    if (!lerLinha("Digite o ID da consulta (ex: 001) [vazio = cancelar]: ", id)) return false;
    id = trim(id);
    if (id.empty()) { std::cout << "\nOperação cancelada.\n"; return false; }
    if (id.find_first_of(" \t") != std::string::npos || tamanhoUtf8(id) > 20) {
        std::cout << "\n" << VERMELHO << "ID inválido:" << RESET << " use até 20 caracteres, sem espaços.\n";
        return false;
    }
    if (!lerLinha("Digite o Nome do paciente: ", nome)) return false;
    nome = trim(nome);
    if (nome.empty() || tamanhoUtf8(nome) > 100) {
        std::cout << "\n" << VERMELHO << "Nome inválido:" << RESET << " informe de 1 a 100 caracteres.\n";
        return false;
    }

    // O ID identifica um paciente: não pode repetir na mesma estrutura nem
    // aparecer em outra com um nome diferente (o banco guarda um nome por ID).
    for (const Consulta& x : pilhaDado.paraVetor()) {
        if (x.id != id) continue;
        if (paraPilha) { std::cout << "\n" << VERMELHO << "O ID '" << id << "' já está na pilha." << RESET << "\n"; return false; }
        if (x.nome != nome) { std::cout << "\n" << VERMELHO << "O ID '" << id << "' já pertence a outro paciente (" << x.nome << ")." << RESET << "\n"; return false; }
    }
    for (const Consulta& x : filaDado.paraVetor()) {
        if (x.id != id) continue;
        if (!paraPilha) { std::cout << "\n" << VERMELHO << "O ID '" << id << "' já está na fila." << RESET << "\n"; return false; }
        if (x.nome != nome) { std::cout << "\n" << VERMELHO << "O ID '" << id << "' já pertence a outro paciente (" << x.nome << ")." << RESET << "\n"; return false; }
    }
    c = {id, nome};
    return true;
}

// ---------------------------------------------------------------------------
// Ações
// ---------------------------------------------------------------------------
void ClinicaMenu::pushConsultas() {
    Consulta c;
    if (!lerConsulta(c, true)) return;
    pilhaDado.push(c.id, c.nome);
    alterado_ = true;
    std::cout << "\n" << VERDE << "Consulta adicionada com sucesso!" << RESET << "\n";
}

void ClinicaMenu::removeConsultas() {
    Consulta c;
    if (!pilhaDado.pop(c)) { std::cout << "A pilha está vazia.\n"; return; }
    alterado_ = true;
    std::cout << "Consulta removida: " << c.id << " | " << c.nome << "\n";
}

void ClinicaMenu::showConsultas() {
    std::cout << CIANO << NEGRITO << "=== Pilha de Consultas ===" << RESET << "\n";
    if (pilhaDado.empty()) std::cout << "  (vazia)\n";
    bool topo = true;
    for (const Node* n = pilhaDado.head.get(); n; n = n->next.get()) {
        std::cout << "  - " << n->dado.id << " | " << n->dado.nome;
        if (topo) std::cout << VERDE << "   <- topo" << RESET;
        std::cout << "\n";
        topo = false;
    }
    std::cout << "\n";
    mostrarFila();
}

void ClinicaMenu::mostrarFila() {
    std::cout << CIANO << NEGRITO << "=== Fila de Atendimento ===" << RESET << "\n";
    if (filaDado.empty()) { std::cout << "  (vazia)\n"; return; }
    int pos = 1;
    for (const Node* n = filaDado.head.get(); n; n = n->next.get(), ++pos) {
        std::cout << "  " << pos << ". " << n->dado.id << " | " << n->dado.nome;
        if (pos == 1) std::cout << VERDE << "   <- próximo" << RESET;
        std::cout << "\n";
    }
}

void ClinicaMenu::adicionarFila() {
    Consulta c;
    if (!lerConsulta(c, false)) return;
    filaDado.push_back(c.id, c.nome);
    alterado_ = true;
    std::cout << "\n" << VERDE << "Paciente adicionado à fila (posição " << filaDado.size() << ")." << RESET << "\n";
}

void ClinicaMenu::loadData() {
    if (!storage_) { std::cout << VERMELHO << "Nenhum armazenamento configurado." << RESET << "\n"; return; }
    if (alterado_ && !confirmar(std::string(AMARELO) + "Há alterações não salvas que serão perdidas. Continuar?" + RESET)) {
        std::cout << "Operação cancelada.\n";
        return;
    }
    std::vector<Consulta> pilha, fila;
    std::string erro;
    std::cout << "Carregando dados do " << storage_->nome() << "...\n";
    if (!storage_->carregar(pilha, fila, erro)) {
        std::cout << VERMELHO << "Erro ao carregar:" << RESET << " " << erro << "\n";
        return;
    }
    pilhaDado.reset();
    filaDado.reset();
    // A pilha vem do topo para a base: empilha de trás para frente.
    for (auto it = pilha.rbegin(); it != pilha.rend(); ++it) pilhaDado.push(it->id, it->nome);
    for (const auto& c : fila) filaDado.push_back(c.id, c.nome);
    alterado_ = false;
    std::cout << VERDE << "Dados recuperados do " << storage_->nome() << " com sucesso!" << RESET
              << "  (" << pilhaDado.size() << " na pilha, " << filaDado.size() << " na fila)\n";
}

void ClinicaMenu::saveData() {
    if (!storage_) { std::cout << VERMELHO << "Nenhum armazenamento configurado." << RESET << "\n"; return; }
    std::string erro;
    std::cout << "Sincronizando dados com o " << storage_->nome() << "...\n";
    if (!storage_->salvar(pilhaDado.paraVetor(), filaDado.paraVetor(), erro)) {
        std::cout << VERMELHO << "Erro ao salvar:" << RESET << " " << erro << "\n";
        return;
    }
    alterado_ = false;
    std::cout << VERDE << "Dados salvos no " << storage_->nome() << " com sucesso!" << RESET << "\n";
}

// ---------------------------------------------------------------------------
// Submenus
// ---------------------------------------------------------------------------
void ClinicaMenu::managePilha() {
    int atual = 0;
    const std::vector<std::string> opcoes = {
        "1. Empilhar consulta",
        "2. Desempilhar (remover topo)",
        "3. Ver topo da pilha",
        "4. Esvaziar pilha inteira",
        "5. Voltar ao menu principal",
    };
    while (true) {
        int e = selecionar("GERENCIAR PILHA", opcoes, atual, true);
        if (e < 0 || e == 4) return;
        limparTela();
        switch (e) {
            case 0: pushConsultas(); break;
            case 1: removeConsultas(); break;
            case 2:
                if (const Consulta* t = pilhaDado.top()) std::cout << "Topo da pilha: " << t->id << " | " << t->nome << "\n";
                else std::cout << "Pilha vazia.\n";
                break;
            case 3: {
                if (pilhaDado.empty()) { std::cout << "Pilha vazia.\n"; break; }
                Consulta c;
                while (pilhaDado.pop(c)) std::cout << "Paciente " << c.id << " | " << c.nome << " saiu da pilha.\n";
                alterado_ = true;
                break;
            }
        }
        std::cout << "\nPressione " << VERDE << "qualquer tecla" << RESET << " para continuar...";
        std::cout.flush();
        lerTecla();
    }
}

void ClinicaMenu::manageFila() {
    int atual = 0;
    const std::vector<std::string> opcoes = {
        "1. Adicionar paciente à fila",
        "2. Chamar próximo (atender)",
        "3. Ver próximo da fila",
        "4. Mostrar fila",
        "5. Esvaziar fila inteira",
        "6. Voltar ao menu principal",
    };
    while (true) {
        int e = selecionar("GERENCIAR FILA", opcoes, atual, true);
        if (e < 0 || e == 5) return;
        limparTela();
        switch (e) {
            case 0: adicionarFila(); break;
            case 1: {
                Consulta c;
                if (filaDado.pop_front(c)) { alterado_ = true; std::cout << "Atendendo agora: " << c.id << " | " << c.nome << "\n"; }
                else std::cout << "Fila vazia.\n";
                break;
            }
            case 2:
                if (const Consulta* f = filaDado.front()) std::cout << "Próximo da fila: " << f->id << " | " << f->nome << "\n";
                else std::cout << "Fila vazia.\n";
                break;
            case 3: mostrarFila(); break;
            case 4: {
                if (filaDado.empty()) { std::cout << "Fila vazia.\n"; break; }
                Consulta c;
                while (filaDado.pop_front(c)) std::cout << "Paciente " << c.id << " | " << c.nome << " saiu da fila.\n";
                alterado_ = true;
                break;
            }
        }
        std::cout << "\nPressione " << VERDE << "qualquer tecla" << RESET << " para continuar...";
        std::cout.flush();
        lerTecla();
    }
}
