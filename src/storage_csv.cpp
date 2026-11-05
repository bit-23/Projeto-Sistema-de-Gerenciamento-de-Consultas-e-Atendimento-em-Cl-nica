// Persistência em arquivo CSV: estrutura,id,nome  (estrutura = pilha | fila)
#include <cstdio>
#include <fstream>
#include <sstream>

#include "storage.h"

namespace {

std::string escapar(const std::string& s) {
    if (s.find_first_of(",\"\n\r") == std::string::npos) return s;
    std::string r = "\"";
    for (char c : s) { if (c == '"') r += '"'; r += c; }
    return r + "\"";
}

// Divide uma linha CSV respeitando aspas.
std::vector<std::string> dividir(const std::string& linha) {
    std::vector<std::string> campos;
    std::string atual;
    bool aspas = false;
    for (std::size_t i = 0; i < linha.size(); ++i) {
        char c = linha[i];
        if (aspas) {
            if (c == '"') {
                if (i + 1 < linha.size() && linha[i + 1] == '"') { atual += '"'; ++i; }
                else aspas = false;
            } else atual += c;
        } else if (c == '"') aspas = true;
        else if (c == ',') { campos.push_back(atual); atual.clear(); }
        else atual += c;
    }
    campos.push_back(atual);
    return campos;
}

class CsvStorage : public Storage {
public:
    explicit CsvStorage(std::string caminho) : caminho_(std::move(caminho)) {}
    std::string nome() const override { return "CSV"; }

    bool salvar(const std::vector<Consulta>& pilha,
                const std::vector<Consulta>& fila, std::string& erro) override {
        // Grava em arquivo temporário e renomeia: nunca deixa o arquivo pela metade.
        const std::string tmp = caminho_ + ".tmp";
        {
            std::ofstream f(tmp, std::ios::trunc);
            if (!f) { erro = "não foi possível escrever em " + tmp; return false; }
            f << "estrutura,id,nome\n";
            for (const auto& c : pilha) f << "pilha," << escapar(c.id) << ',' << escapar(c.nome) << '\n';
            for (const auto& c : fila)  f << "fila,"  << escapar(c.id) << ',' << escapar(c.nome) << '\n';
            f.flush();
            if (!f) { erro = "falha ao gravar " + tmp; return false; }
        }
        std::remove(caminho_.c_str());   // no Windows, rename não sobrescreve
        if (std::rename(tmp.c_str(), caminho_.c_str()) != 0) {
            erro = "não foi possível substituir " + caminho_;
            return false;
        }
        return true;
    }

    bool carregar(std::vector<Consulta>& pilha,
                  std::vector<Consulta>& fila, std::string& erro) override {
        std::ifstream f(caminho_);
        if (!f) { erro = "arquivo '" + caminho_ + "' não encontrado (salve primeiro)"; return false; }
        pilha.clear(); fila.clear();
        std::string linha;
        std::getline(f, linha);                       // cabeçalho
        while (std::getline(f, linha)) {
            if (!linha.empty() && linha.back() == '\r') linha.pop_back();
            if (linha.empty()) continue;
            auto c = dividir(linha);
            if (c.size() < 3) continue;
            if (c[0] == "pilha")     pilha.push_back({c[1], c[2]});
            else if (c[0] == "fila") fila.push_back({c[1], c[2]});
        }
        return true;
    }

private:
    std::string caminho_;
};

} // namespace

std::unique_ptr<Storage> criarStorageCsv(const std::string& caminho) {
    return std::make_unique<CsvStorage>(caminho);
}
