#include "terminal.h"

#include <iostream>

#ifdef _WIN32
  #ifndef NOMINMAX
  #define NOMINMAX
  #endif
  #include <windows.h>
  #include <conio.h>
#else
  #include <poll.h>
  #include <termios.h>
  #include <unistd.h>
#endif

namespace term {

static bool g_ascii = false;
void setModoAscii(bool ativo) { g_ascii = ativo; }
bool modoAscii() { return g_ascii; }

void limparTela() {
    std::cout << "\033[2J\033[1;1H" << std::flush;
}

#ifdef _WIN32

void iniciar() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo = 0;
    if (out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &modo)) {
        // Sem isso, o cmd/PowerShell antigo imprime "←[36m" em vez de cores.
        SetConsoleMode(out, modo | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}

Entrada lerTecla() {
    Entrada e;
    int c = _getch();
    if (c > 0 && c < 128) e.ch = (char)c;
    if (c == 0 || c == 224) {            // tecla especial: o código vem a seguir
        int k = _getch();
        if (k == 72) e.tecla = Tecla::Cima;
        else if (k == 80) e.tecla = Tecla::Baixo;
        return e;
    }
    if (c == 13 || c == 10) e.tecla = Tecla::Enter;
    else if (c == 27)       e.tecla = Tecla::Esc;
    else if (c >= '0' && c <= '9') { e.tecla = Tecla::Numero; e.numero = c - '0'; }
    return e;
}

#else

void iniciar() {}

namespace {
// Modo "raw" apenas durante a leitura de uma tecla; restaurado ao sair.
struct ModoRaw {
    termios antigo{};
    bool ok = false;
    ModoRaw() {
        if (tcgetattr(STDIN_FILENO, &antigo) == 0) {
            termios novo = antigo;
            novo.c_lflag &= ~(ICANON | ECHO);
            novo.c_cc[VMIN] = 1;
            novo.c_cc[VTIME] = 0;
            ok = tcsetattr(STDIN_FILENO, TCSANOW, &novo) == 0;
        }
    }
    ~ModoRaw() { if (ok) tcsetattr(STDIN_FILENO, TCSADRAIN, &antigo); }
};

bool temByte(int ms) {
    pollfd p{STDIN_FILENO, POLLIN, 0};
    return poll(&p, 1, ms) > 0;
}
} // namespace

Entrada lerTecla() {
    Entrada e;
    ModoRaw raw;
    unsigned char c = 0;
    if (read(STDIN_FILENO, &c, 1) <= 0) { e.tecla = Tecla::Esc; return e; } // EOF
    if (c < 128) e.ch = (char)c;
    if (c == 27) {
        // Sequência de seta: ESC [ A  ou  ESC O A. ESC sozinho = cancelar.
        if (!temByte(50)) { e.tecla = Tecla::Esc; return e; }
        unsigned char b = 0, d = 0;
        if (read(STDIN_FILENO, &b, 1) <= 0) return e;
        if ((b == '[' || b == 'O') && temByte(50) && read(STDIN_FILENO, &d, 1) > 0) {
            if (d == 'A') e.tecla = Tecla::Cima;
            else if (d == 'B') e.tecla = Tecla::Baixo;
            else while (d >= '0' && d <= '?' && temByte(5) && read(STDIN_FILENO, &d, 1) > 0) {}
        }
        return e;
    }
    if (c == '\n' || c == '\r') e.tecla = Tecla::Enter;
    else if (c >= '0' && c <= '9') { e.tecla = Tecla::Numero; e.numero = c - '0'; }
    return e;
}

#endif

bool lerLinha(const std::string& prompt, std::string& saida) {
    std::cout << prompt << std::flush;
    if (!std::getline(std::cin, saida)) return false;
    if (!saida.empty() && saida.back() == '\r') saida.pop_back();
    return true;
}

} // namespace term
