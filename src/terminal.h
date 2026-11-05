// Camada de terminal multiplataforma (Linux, macOS e Windows).
#ifndef TERMINAL_H
#define TERMINAL_H

#include <string>

namespace term {

enum class Tecla { Cima, Baixo, Enter, Esc, Numero, Outra };

struct Entrada {
    Tecla tecla = Tecla::Outra;
    int numero = 0;   // válido quando tecla == Numero (0-9)
    char ch = 0;      // caractere ASCII digitado (0 se não houver)
};

// Prepara o terminal: ativa cores ANSI e UTF-8 (necessário no Windows).
void iniciar();

// Lê uma tecla sem eco e sem precisar de Enter. Setas viram Cima/Baixo.
Entrada lerTecla();

// Lê uma linha completa (aceita espaços). Retorna false em EOF.
bool lerLinha(const std::string& prompt, std::string& saida);

void limparTela();

// Usa ">", "^" e "v" em vez de ➔ ▲ ▼ (para consoles sem suporte a Unicode).
void setModoAscii(bool ativo);
bool modoAscii();

// Códigos de cor ANSI.
inline constexpr const char* RESET   = "\033[0m";
inline constexpr const char* NEGRITO = "\033[1m";
inline constexpr const char* CIANO   = "\033[36m";
inline constexpr const char* VERDE   = "\033[32m";
inline constexpr const char* VERMELHO = "\033[1;31m";
inline constexpr const char* AMARELO = "\033[33m";
inline constexpr const char* REVERSO = "\033[7m";

} // namespace term

#endif // TERMINAL_H
