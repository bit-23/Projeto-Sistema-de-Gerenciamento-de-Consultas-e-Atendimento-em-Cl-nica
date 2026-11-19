// =====================================================================
// SISTEMA DE GERENCIAMENTO DE CONSULTAS E ATENDIMENTO EM CLÍNICA
// ---------------------------------------------------------------------
// Estruturas: Lista Duplamente Encadeada (Consultas), Fila (Atendimento), Pilha (Histórico)
// Persistência: Arquivo CSV simples (sem banco de dados)
// =====================================================================

#include <iostream>
#include <string>
#include <limits>
#include <fstream>   // Para ler e escrever arquivos (CSV)
#include <sstream>   // Para separar as linhas do CSV por vírgula

using namespace std;

// ---------------------------------------------------------------------
// 1. NÓS DAS ESTRUTURAS DE DADOS
// ---------------------------------------------------------------------

struct Consulta {
    string paciente;
    string medico;
    string horario;      // formato "HH:MM"
    Consulta* anterior;
    Consulta* proximo;
};

struct Paciente {
    string nome;
    Paciente* proximo;
};

struct Operacao {
    string texto;
    Operacao* proximo;
};

// ---------------------------------------------------------------------
// 2. VARIÁVEIS GLOBAIS (Pontas das estruturas)
// ---------------------------------------------------------------------
Consulta* inicio = nullptr;
Consulta* fim = nullptr;

Paciente* frente = nullptr;
Paciente* tras = nullptr;

Operacao* topo = nullptr;
int totalAtendimentos = 0;

// ---------------------------------------------------------------------
// 3. FUNÇÕES AUXILIARES
// ---------------------------------------------------------------------

string lerTexto(const string& rotulo) {
    string valor;
    cout << rotulo;
    getline(cin, valor);
    return valor;
}

void limparEntrada() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void registrar(const string& texto) {
    Operacao* nova = new Operacao;
    nova->texto = texto;
    nova->proximo = topo;
    topo = nova;
}

int contarConsultas() {
    int qtd = 0;
    for (Consulta* atual = inicio; atual != nullptr; atual = atual->proximo) qtd++;
    return qtd;
}

// Função interna para inserir consulta já ordenada por horário (usada pelo agendamento e pelo carregamento)
void inserirConsultaOrdenada(Consulta* nova) {
    Consulta* atual = inicio;
    while (atual != nullptr && atual->horario <= nova->horario) {
        atual = atual->proximo;
    }

    nova->proximo = atual;
    nova->anterior = (atual != nullptr) ? atual->anterior : fim;

    if (nova->anterior != nullptr) {
        nova->anterior->proximo = nova;
    } else {
        inicio = nova;
    }

    if (atual != nullptr) {
        atual->anterior = nova;
    } else {
        fim = nova;
    }
}

// ---------------------------------------------------------------------
// 4. GERENCIAMENTO DE CONSULTAS (Lista Duplamente Encadeada)
// ---------------------------------------------------------------------

void agendarConsulta() {
    Consulta* nova = new Consulta;
    nova->paciente = lerTexto("  Nome do paciente: ");
    nova->medico   = lerTexto("  Nome do medico:   ");
    nova->horario  = lerTexto("  Horario (HH:MM):  ");
    
    inserirConsultaOrdenada(nova);
    cout << "  Consulta agendada com sucesso!\n";
    registrar("Agendou " + nova->paciente + " as " + nova->horario);
}

void cancelarConsulta() {
    string nome = lerTexto("  Nome do paciente a cancelar: ");
    Consulta* atual = inicio;

    while (atual != nullptr) {
        if (atual->paciente == nome) {
            if (atual->anterior != nullptr) atual->anterior->proximo = atual->proximo;
            else inicio = atual->proximo;

            if (atual->proximo != nullptr) atual->proximo->anterior = atual->anterior;
            else fim = atual->anterior;

            delete atual;
            cout << "  Consulta cancelada.\n";
            registrar("Cancelou consulta de " + nome);
            return;
        }
        atual = atual->proximo;
    }
    cout << "  Consulta nao encontrada.\n";
}

void listarConsultas() {
    cout << "\n  CONSULTAS AGENDADAS (por horario)\n";
    if (inicio == nullptr) { cout << "  (nenhuma consulta)\n"; return; }
    
    for (Consulta* atual = inicio; atual != nullptr; atual = atual->proximo) {
        cout << "  [" << atual->horario << "] " << atual->paciente << " com Dr(a). " << atual->medico << "\n";
    }
}

// ---------------------------------------------------------------------
// 5. GERENCIAMENTO DE FILA (FIFO)
// ---------------------------------------------------------------------

void entrarNaFila() {
    Paciente* novo = new Paciente;
    novo->nome = lerTexto("  Nome do paciente: ");
    novo->proximo = nullptr;

    if (tras == nullptr) { frente = tras = novo; } 
    else { tras->proximo = novo; tras = novo; }
    
    cout << "  Paciente adicionado a fila de espera!\n";
    registrar("Enfileirou " + novo->nome);
}

void chamarProximo() {
    if (frente == nullptr) { cout << "  Fila vazia.\n"; return; }
    
    string nome = frente->nome;
    Paciente* antigo = frente;
    frente = frente->proximo;
    if (frente == nullptr) tras = nullptr;
    
    delete antigo;
    totalAtendimentos++;
    cout << "  Chamando para atendimento: " << nome << "\n";
    registrar("Atendeu " + nome);
}

void mostrarFila() {
    cout << "\n  FILA DE ESPERA\n";
    if (frente == nullptr) { cout << "  (ninguem esperando)\n"; return; }
    
    int pos = 1;
    for (Paciente* atual = frente; atual != nullptr; atual = atual->proximo) {
        cout << "  " << pos++ << ". " << atual->nome << "\n";
    }
}

// ---------------------------------------------------------------------
// 6. HISTÓRICO (PILHA LIFO)
// ---------------------------------------------------------------------

void mostrarHistorico() {
    cout << "\n  HISTORICO DE OPERACOES (mais recente primeiro)\n";
    if (topo == nullptr) { cout << "  (nenhuma operacao registrada)\n"; return; }
    
    for (Operacao* atual = topo; atual != nullptr; atual = atual->proximo) {
        cout << "  -> " << atual->texto << "\n";
    }
}

// ---------------------------------------------------------------------
// 7. PERSISTÊNCIA EM CSV (Simples e Legível)
// ---------------------------------------------------------------------

void salvarCSV() {
    ofstream arquivo("clinica_dados.csv");
    if (!arquivo.is_open()) {
        cout << "  Erro ao abrir o arquivo para salvamento.\n";
        return;
    }

    // Salva Consultas (Prefixo 'C')
    for (Consulta* atual = inicio; atual != nullptr; atual = atual->proximo) {
        arquivo << "C," << atual->paciente << "," << atual->medico << "," << atual->horario << "\n";
    }

    // Salva Fila (Prefixo 'F')
    for (Paciente* atual = frente; atual != nullptr; atual = atual->proximo) {
        arquivo << "F," << atual->nome << "\n";
    }

    arquivo.close();
    cout << "  Dados salvos com sucesso em 'clinica_dados.csv'!\n";
    registrar("Salvou dados no CSV");
}

void carregarCSV() {
    ifstream arquivo("clinica_dados.csv");
    if (!arquivo.is_open()) {
        cout << "  Arquivo 'clinica_dados.csv' nao encontrado ou vazio.\n";
        return;
    }

    // Limpa dados atuais antes de carregar
    while (inicio != nullptr) { Consulta* p = inicio; inicio = inicio->proximo; delete p; }
    while (frente != nullptr) { Paciente* p = frente; frente = frente->proximo; delete p; }
    fim = nullptr; tras = nullptr;
    totalAtendimentos = 0;

    string linha;
    while (getline(arquivo, linha)) {
        stringstream ss(linha);
        string tipo, dado1, dado2, dado3;
        
        getline(ss, tipo, ',');
        getline(ss, dado1, ',');

        if (tipo == "C") {
            getline(ss, dado2, ',');
            getline(ss, dado3, ',');
            Consulta* nova = new Consulta{dado1, dado2, dado3, nullptr, nullptr};
            inserirConsultaOrdenada(nova);
        } 
        else if (tipo == "F") {
            Paciente* novo = new Paciente{dado1, nullptr};
            if (tras == nullptr) { frente = tras = novo; } 
            else { tras->proximo = novo; tras = novo; }
        }
    }
    arquivo.close();
    cout << "  Dados carregados com sucesso do 'clinica_dados.csv'!\n";
    registrar("Carregou dados do CSV");
}

// ---------------------------------------------------------------------
// 8. MENU PRINCIPAL
// ---------------------------------------------------------------------

int main() {
    int opcao = -1;

    do {
        cout << "\n=======================================\n";
        cout << "     SISTEMA DE GESTAO DE CLINICA\n";
        cout << "=======================================\n";
        cout << "  1. Agendar consulta\n";
        cout << "  2. Cancelar consulta\n";
        cout << "  3. Mostrar consultas agendadas\n";
        cout << "  4. Adicionar paciente a fila de espera\n";
        cout << "  5. Chamar proximo paciente (atender)\n";
        cout << "  6. Mostrar fila de espera\n";
        cout << "  7. Gerar relatorio\n";
        cout << "  8. Mostrar historico de operacoes\n";
        cout << "  9. SALVAR dados em arquivo CSV\n";
        cout << " 10. CARREGAR dados de arquivo CSV\n";
        cout << "  0. Sair\n";
        cout << "---------------------------------------\n";
        cout << "  Consultas: " << contarConsultas() << " | Atendimentos: " << totalAtendimentos << "\n";
        cout << "---------------------------------------\n";
        cout << "  Escolha uma opcao: ";
        
        cin >> opcao;
        limparEntrada();

        switch (opcao) {
            case 1: agendarConsulta();  break;
            case 2: cancelarConsulta(); break;
            case 3: listarConsultas();  break;
            case 4: entrarNaFila();     break;
            case 5: chamarProximo();    break;
            case 6: mostrarFila();      break;
            case 7: 
                cout << "\n  ========== RELATORIO ==========\n";
                cout << "  Consultas agendadas: " << contarConsultas() << "\n";
                cout << "  Atendimentos feitos: " << totalAtendimentos << "\n";
                cout << "  =================================\n";
                registrar("Gerou relatorio");
                break;
            case 8: mostrarHistorico(); break;
            case 9: salvarCSV();        break;
            case 10: carregarCSV();     break;
            case 0: cout << "  Encerrando o sistema. Ate logo!\n"; break;
            default: cout << "  Opcao invalida. Tente novamente.\n";
        }
    } while (opcao != 0);

    // Liberação segura de memória antes de sair
    while (inicio != nullptr) { Consulta* p = inicio; inicio = inicio->proximo; delete p; }
    while (frente != nullptr) { Paciente* p = frente; frente = frente->proximo; delete p; }
    while (topo   != nullptr) { Operacao* p = topo;   topo   = topo->proximo;   delete p; }

    return 0;
}
