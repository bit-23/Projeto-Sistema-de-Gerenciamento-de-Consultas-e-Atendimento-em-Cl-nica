// Estruturas de dados da clínica: Pilha (LIFO) e Fila (FIFO) em listas
// simplesmente encadeadas, com gerenciamento automático de memória.
#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Registro de uma consulta / paciente.
struct Consulta {
    std::string id;
    std::string nome;
};

// Nó da lista simplesmente encadeada.
struct Node {
    Consulta dado;
    std::unique_ptr<Node> next;
    explicit Node(Consulta c) : dado(std::move(c)), next(nullptr) {}
};

// Libera a lista de forma iterativa. Sem isso, o destrutor padrão do
// unique_ptr é recursivo e pode estourar a pilha com listas muito grandes.
inline void liberarLista(std::unique_ptr<Node>& head) {
    while (head) {
        std::unique_ptr<Node> prox = std::move(head->next);
        head = std::move(prox);
    }
}

// --- Pilha de Consultas (LIFO) -------------------------------------------
// push / pop / top em O(1).
class ClinicPilha {
public:
    std::unique_ptr<Node> head;

    ClinicPilha() = default;
    ClinicPilha(const ClinicPilha&) = delete;
    ClinicPilha& operator=(const ClinicPilha&) = delete;
    ~ClinicPilha() { reset(); }

    void push(const std::string& id, const std::string& nome) {
        auto novo = std::make_unique<Node>(Consulta{id, nome});
        novo->next = std::move(head);
        head = std::move(novo);
        ++tamanho_;
    }

    // Remove o topo. Retorna false se a pilha estiver vazia.
    bool pop(Consulta& saida) {
        if (!head) return false;
        saida = std::move(head->dado);
        std::unique_ptr<Node> prox = std::move(head->next);
        head = std::move(prox);
        --tamanho_;
        return true;
    }

    const Consulta* top() const { return head ? &head->dado : nullptr; }
    bool empty() const { return head == nullptr; }
    std::size_t size() const { return tamanho_; }

    void reset() { liberarLista(head); tamanho_ = 0; }

    // Elementos do topo para a base.
    std::vector<Consulta> paraVetor() const {
        std::vector<Consulta> v;
        v.reserve(tamanho_);
        for (const Node* n = head.get(); n; n = n->next.get()) v.push_back(n->dado);
        return v;
    }

private:
    std::size_t tamanho_ = 0;
};

// --- Fila de Atendimento (FIFO) ------------------------------------------
// Mantém ponteiro para o último nó: enfileirar é O(1) (antes era O(n)).
class ClinicFila {
public:
    std::unique_ptr<Node> head;

    ClinicFila() = default;
    ClinicFila(const ClinicFila&) = delete;
    ClinicFila& operator=(const ClinicFila&) = delete;
    ~ClinicFila() { reset(); }

    void push_back(const std::string& id, const std::string& nome) {
        auto novo = std::make_unique<Node>(Consulta{id, nome});
        Node* raw = novo.get();
        if (!head) head = std::move(novo);
        else       tail_->next = std::move(novo);
        tail_ = raw;
        ++tamanho_;
    }

    // Remove o primeiro da fila. Retorna false se estiver vazia.
    bool pop_front(Consulta& saida) {
        if (!head) return false;
        saida = std::move(head->dado);
        std::unique_ptr<Node> prox = std::move(head->next);
        head = std::move(prox);
        if (!head) tail_ = nullptr;
        --tamanho_;
        return true;
    }

    const Consulta* front() const { return head ? &head->dado : nullptr; }
    bool empty() const { return head == nullptr; }
    std::size_t size() const { return tamanho_; }

    void reset() { liberarLista(head); tail_ = nullptr; tamanho_ = 0; }

    // Elementos do primeiro ao último.
    std::vector<Consulta> paraVetor() const {
        std::vector<Consulta> v;
        v.reserve(tamanho_);
        for (const Node* n = head.get(); n; n = n->next.get()) v.push_back(n->dado);
        return v;
    }

private:
    Node* tail_ = nullptr;
    std::size_t tamanho_ = 0;
};

#endif // ESTRUTURAS_H
