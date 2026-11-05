#include <mysql/mysql.h>
#include "clinica.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

// Incluindo bibliotecas nativas do Linux para capturar teclas sem precisar de Enter
#include <termios.h>
#include <unistd.h>

// Função para persistir e sincronizar as estruturas locais (Pilha/Fila) com as tabelas do MySQL
void ClinicaMenu::saveDataCSV() { // Mantido o nome do método para compatibilidade com o Menu
    MYSQL* conn = mysql_init(nullptr);
    
    // Conecta no MariaDB dentro do Podman (IP, Usuário, Senha, Nome_do_Banco, Porta)
    if (!mysql_real_connect(conn, "100.120.87.102", "root", "Suasenha", "Sistema_de_Gerenciamento_de_consultas", 3306, nullptr, 0)) {
        std::cerr << "\033[1;31mErro ao conectar no banco do Podman:\033[0m " << mysql_error(conn) << std::endl;
        return;
    }

    std::cout << "Sincronizando dados com o MariaDB...\n";

    // 1. Limpa os registros de ordenação antigos para reinserir o estado atualizado das estruturas
    mysql_query(conn, "DELETE FROM pilha_consultas");
    mysql_query(conn, "DELETE FROM fila_atendimento");

    // Helper Lambda para tratar a string "id | nome" e extrair campos puros
    auto parseAndInsert = [&](Node* node, const std::string& tabela) {
        while (node != nullptr) {
            std::stringstream ss(node->data);
            std::string id, lixo, nome;
            ss >> id >> lixo; // Extrai o "id" e o caractere "|"
            std::getline(ss >> std::ws, nome); // Pega o resto da string que contém o nome

            if (!id.empty() && !nome.empty()) {
                // Insere ou atualiza o cadastro mestre do paciente
                std::string queryPaciente = "INSERT INTO pacientes (id, nome) VALUES ('" + id + "', '" + nome + "') "
                                            "ON DUPLICATE KEY UPDATE nome='" + nome + "'";
                mysql_query(conn, queryPaciente.c_str());

                // Vincula o paciente à respectiva tabela estrutural (Pilha ou Fila)
                std::string queryEstrutura = "INSERT INTO " + tabela + " (paciente_id) VALUES ('" + id + "')";
                mysql_query(conn, queryEstrutura.c_str());
            }
            node = node->next.get();
        }
    };

    // Salva o conteúdo atual da Pilha local no banco
    parseAndInsert(pilhaDado.head.get(), "pilha_consultas");

    // Salva o conteúdo atual da Fila local no banco
    parseAndInsert(filaDado.head.get(), "fila_atendimento");
    
    mysql_close(conn);
    std::cout << "\033[1;32mDados salvos no MySQL com sucesso!\033[0m" << std::endl;
}

// Função para buscar e recarregar os dados gravados no MySQL de volta para a memória do programa
void ClinicaMenu::loadDataCSV(const std::string& filename) { // Mantido assinatura para compatibilidade
    MYSQL* conn = mysql_init(nullptr);
    
    if (!mysql_real_connect(conn, "127.0.0.1", "root", "Suasenha", "Sistema_de_Gerenciamento_de_consultas", 3306, nullptr, 0)) {
        std::cerr << "\033[1;31mErro ao conectar no banco do Podman:\033[0m " << mysql_error(conn) << std::endl;
        return;
    }

    // Limpa a memória das estruturas locais antes do carregamento
    pilhaDado.reset();
    filaDado.reset();

    // 1. Carrega dados salvos na Pilha (Inverte a ordem para recuperar o comportamento LIFO correto)
    std::string sqlPilha = "SELECT p.id, p.nome FROM pilha_consultas c JOIN pacientes p ON c.paciente_id = p.id ORDER BY c.id_registro DESC";
    if (mysql_query(conn, sqlPilha.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                pilhaDado.push(row[0], row[1]);
            }
            mysql_free_result(res);
        }
    }

    // 2. Carrega dados salvos na Fila (Mantém a ordenação cronológica correta)
    std::string sqlFila = "SELECT p.id, p.nome FROM fila_atendimento f JOIN pacientes p ON f.paciente_id = p.id ORDER BY f.id_registro ASC";
    if (mysql_query(conn, sqlFila.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                filaDado.push_front(row[0], row[1]);
            }
            mysql_free_result(res);
        }
    }

    mysql_close(conn);
    std::cout << "\033[1;32mDados recuperados do MariaDB com sucesso!\033[0m" << std::endl;
}

// Função auxiliar para ler um caractere do teclado imediatamente (sem Echo e sem Enter)
char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0) perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ~ICANON");
    if (read(0, &buf, 1) < 0) perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0) perror ("tcsetattr ICANON");
    return (buf);
}

// Função para desenhar o menu interativo estilo archinstall
int selecionarOpcaoMenu(int destaque) {
    const std::string RESET   = "\033[0m";
    const std::string NEGRITO = "\033[1m";
    const std::string CIANO   = "\033[36m";
    const std::string VERDE   = "\033[32m";
    const std::string FUNDO_SELECIONADO = "\033[7m";

    std::cout << "\033[2J\033[1;1H";

    std::cout << CIANO << NEGRITO << "=======================================" << RESET << std::endl;
    std::cout << CIANO << NEGRITO << "       SISTEMA DE GESTÃO CLÍNICA       " << RESET << std::endl;
    std::cout << CIANO << NEGRITO << "=======================================" << RESET << std::endl;
    std::cout << "Use as " << VERDE << "setas (▲/▼)" << RESET << " para navegar e " << VERDE << "Enter" << RESET << " para confirmar:\n\n";

    std::string opcoes[] = {
        "1. Adicionar Consulta (Pilha)",
        "2. Remover Consulta (Pilha)",
        "3. Mostrar Consultas",
        "4. Carregar Dados do MariaDB",
        "5. Salvar Dados no MariaDB",
        "6. Gerenciar Pilha",
        "7. Gerenciar Fila",
        "8. Sair do Programa"
    };

    for (int i = 0; i < 8; i++) {
        if (i == destaque) {
            std::cout << FUNDO_SELECIONADO << VERDE << NEGRITO << "  ➔  " << opcoes[i] << "  " << RESET << std::endl;
        } else {
            std::cout << "     " << opcoes[i] << std::endl;
        }
    }
    std::cout << "\n" << CIANO << "=======================================" << RESET << std::endl;

    return 0;
}

int main() {
    ClinicaMenu menu;
    
    menu.pilhaDado.reset(); 
    menu.filaDado.reset();

    int opcaoAtual = 0;

    while (true) {
        selecionarOpcaoMenu(opcaoAtual);
        char c = getch();

        if (c == 27) { 
            getch(); 
            char direcao = getch();
            if (direcao == 65) { 
                opcaoAtual = (opcaoAtual - 1 + 8) % 8;
            } else if (direcao == 66) { 
                opcaoAtual = (opcaoAtual + 1) % 8;
            }
        } 
        else if (c == 10) { 
            int escolha = opcaoAtual + 1;

            if (escolha == 8) {
                std::cout << "\nSaindo do programa de forma segura...\n";
                break;
            }

            std::cout << "\033[2J\033[1;1H";
            std::cout << "\033[1;36m[Executando ação...]\033[0m\n\n";

            switch (escolha) {
                case 1: menu.pushConsultas(); break;
                case 2: menu.removeConsultas(); break;
                case 3: menu.showConsultas(); break;
                case 4: menu.loadDataCSV("clinic.csv"); break;
                case 5: menu.saveDataCSV(); break; 
                case 6: menu.managePilha(); break;
                case 7: menu.manageFila(); break;
            }

            std::cout << "\nPressione \033[1;32mqualquer tecla\033[0m para voltar ao menu principal...";
            std::cout.flush();
            getch(); 
        }
    }
    return 0;
}

// --- Submenús específicos ---

void ClinicaMenu::pushConsultas() {
    std::string id, nome;
    std::cout << "Digite o ID da consulta (ex: 001): "; std::cin >> id;
    std::cout << "Digite o Nome do paciente: "; std::cin >> nome;
    
    pilhaDado.push(id, nome); 
    std::cout << "\nConsulta adicionada com sucesso!" << std::endl;
}

void ClinicaMenu::removeConsultas() {
    if (pilhaDado.empty()) {
        std::cout << "A pilha está vazia." << std::endl;
        return;
    }
    
    std::string info = pilhaDado.pop();
    std::cout << "Consulta removida: " << info << std::endl;
}

void ClinicaMenu::showConsultas() {
    if (pilhaDado.empty()) {
        std::cout << "Pilha vazia. Adicione consultas para ver." << std::endl;
        return;
    }
    
    std::cout << "=== Pilha de Consultas ===" << std::endl;
    Node* temp = pilhaDado.head.get();
    while (temp != nullptr) {
        std::cout << "  - " << temp->data << std::endl;
        temp = temp->next.get();
    }
}

void ClinicaMenu::managePilha() {
    if (pilhaDado.empty()) {
        std::cout << "Pilha vazia." << std::endl;
        return;
    }
    
    while (!pilhaDado.empty()) {
        std::string info = pilhaDado.pop();
        std::cout << "Paciente " << info << " saiu da pilha." << std::endl;
    }
}

void ClinicaMenu::manageFila() {
    if (filaDado.empty()) {
        std::cout << "Fila vazia." << std::endl;
        return;
    }
    
    while (!filaDado.empty()) {
        std::string info = filaDado.pop_back(); 
        std::cout << "Paciente " << info << " saiu da fila." << std::endl;
    }
}
