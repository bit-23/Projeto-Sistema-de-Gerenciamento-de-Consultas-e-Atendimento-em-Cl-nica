#ifndef CLINICA_H
#define CLINICA_H

#include <iostream>
#include <string>
#include <memory>

// --- Lista Simplesmente Encadeada ---
class Node {
public:
    std::string data;
    std::unique_ptr<Node> next; // Ponteiro inteligente para o próximo elemento

    Node() : data(""), next(nullptr) {}
    Node(const std::string& str) : data(str), next(nullptr) {}
};

// --- Pilha de Consultas (LIFO) ---
class ClinicPilha {
public:
    std::unique_ptr<Node> head; // Gerencia os nós diretamente

    ClinicPilha() : head(nullptr) {}
    ~ClinicPilha() = default;

    void push(const std::string& id, const std::string& nome) {
        auto newNode = std::make_unique<Node>(id + " | " + nome);
        if (head != nullptr) {
            newNode->next = std::move(head);
        }
        head = std::move(newNode);
    }

    std::string pop() {
        if (head == nullptr) return "";
        std::string val = head->data;
        head = std::move(head->next);
        return val;
    }

    std::string top() const {
        if (head == nullptr) return "Vazia";
        return head->data;
    }

    bool empty() const { return head == nullptr; }
    
    void reset() { head.reset(); }
};

// --- Fila de Atendimento (FIFO) ---
class ClinicFila {
public:
    std::unique_ptr<Node> head;

    ClinicFila() : head(nullptr) {}
    ~ClinicFila() = default;

    void push_front(const std::string& id, const std::string& nome) {
        auto newNode = std::make_unique<Node>(id + " | " + nome);
        if (head == nullptr) {
            head = std::move(newNode);
        } else {
            Node* temp = head.get();
            while (temp->next != nullptr) {
                temp = temp->next.get();
            }
            temp->next = std::move(newNode);
        }
    }

    std::string pop_back() {
        if (head == nullptr) return "";
        std::string val = head->data;
        head = std::move(head->next);
        return val;
    }

    std::string top() const {
        if (head == nullptr) return "Vazia";
        return head->data;
    }

    bool empty() const { return head == nullptr; }
    
    void reset() { head.reset(); }
};

// --- Classe do Menu Principal (Faltava declarar no .h) ---
class ClinicaMenu {
public:
    ClinicPilha pilhaDado;
    ClinicFila filaDado;

    void printMenu() {
        std::cout << "\n=== MENU CLINICA ===" << std::endl;
        std::cout << "1. Adicionar Consulta (Pilha)" << std::endl;
        std::cout << "2. Remover Consulta (Pilha)" << std::endl;
        std::cout << "3. Mostrar Consultas" << std::endl;
        std::cout << "4. Carregar Dados CSV" << std::endl;
        std::cout << "5. Salvar Dados CSV" << std::endl;
        std::cout << "6. Gerenciar Pilha" << std::endl;
        std::cout << "7. Gerenciar Fila" << std::endl;
        std::cout << "8. Sair" << std::endl;
    }

    void pushConsultas();
    void removeConsultas();
    void showConsultas();
    void managePilha();
    void manageFila();
    void loadDataCSV(const std::string& filename);
    void saveDataCSV();
};

#endif