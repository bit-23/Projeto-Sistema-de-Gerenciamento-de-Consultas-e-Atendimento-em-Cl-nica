// Persistência em MariaDB/MySQL. Compilado apenas com -DHAVE_MARIADB.
#include "storage.h"

#ifdef HAVE_MARIADB

#include <mysql.h>

#include <cctype>
#include <cstring>

namespace {

const char* kSchema[] = {
    "CREATE TABLE IF NOT EXISTS pacientes ("
    "  id   VARCHAR(20)  NOT NULL PRIMARY KEY,"
    "  nome VARCHAR(100) NOT NULL"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4",

    "CREATE TABLE IF NOT EXISTS pilha_consultas ("
    "  id_registro INT AUTO_INCREMENT PRIMARY KEY,"
    "  paciente_id VARCHAR(20) NOT NULL,"
    "  FOREIGN KEY (paciente_id) REFERENCES pacientes(id) ON UPDATE CASCADE"
    ") ENGINE=InnoDB",

    "CREATE TABLE IF NOT EXISTS fila_atendimento ("
    "  id_registro INT AUTO_INCREMENT PRIMARY KEY,"
    "  paciente_id VARCHAR(20) NOT NULL,"
    "  FOREIGN KEY (paciente_id) REFERENCES pacientes(id) ON UPDATE CASCADE"
    ") ENGINE=InnoDB",
};

// Conexão com fechamento automático (RAII).
class Conexao {
public:
    ~Conexao() { if (c_) mysql_close(c_); }

    bool abrir(const Config& cfg, std::string& erro) {
        c_ = mysql_init(nullptr);
        if (!c_) { erro = "falha ao inicializar o cliente MySQL"; return false; }
        unsigned timeout = 5;
        mysql_options(c_, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
        mysql_options(c_, MYSQL_SET_CHARSET_NAME, "utf8mb4");

        // Conecta sem banco para poder criá-lo se ainda não existir.
        if (!mysql_real_connect(c_, cfg.dbHost.c_str(), cfg.dbUser.c_str(), cfg.dbSenha.c_str(),
                                nullptr, cfg.dbPorta, nullptr, 0)) {
            erro = mysql_error(c_);
            return false;
        }
        std::string nomeBanco = cfg.dbNome;
        // Nome de banco não pode ser parametrizado: valida e usa crases.
        for (char ch : nomeBanco)
            if (!(isalnum((unsigned char)ch) || ch == '_')) { erro = "nome de banco inválido: " + nomeBanco; return false; }
        if (!exec("CREATE DATABASE IF NOT EXISTS `" + nomeBanco + "` CHARACTER SET utf8mb4", erro)) return false;
        if (mysql_select_db(c_, nomeBanco.c_str()) != 0) { erro = mysql_error(c_); return false; }
        for (const char* ddl : kSchema) if (!exec(ddl, erro)) return false;
        return true;
    }

    bool exec(const std::string& sql, std::string& erro) {
        if (mysql_query(c_, sql.c_str()) != 0) { erro = mysql_error(c_); return false; }
        return true;
    }

    MYSQL* raw() { return c_; }

private:
    MYSQL* c_ = nullptr;
};

// Statement preparado com 1 ou 2 parâmetros string (protege contra SQL injection).
class Stmt {
public:
    Stmt(MYSQL* c, const char* sql, unsigned nParams) : n_(nParams) {
        s_ = mysql_stmt_init(c);
        if (s_ && mysql_stmt_prepare(s_, sql, (unsigned long)std::strlen(sql)) != 0) {
            erro_ = mysql_stmt_error(s_);
            mysql_stmt_close(s_); s_ = nullptr;
        } else if (!s_) erro_ = "falha em mysql_stmt_init";
    }
    ~Stmt() { if (s_) mysql_stmt_close(s_); }
    bool ok() const { return s_ != nullptr; }
    const std::string& erro() const { return erro_; }

    bool executar(const std::string& a, const std::string& b = "") {
        MYSQL_BIND bind[2];
        std::memset(bind, 0, sizeof(bind));
        unsigned long len[2] = {(unsigned long)a.size(), (unsigned long)b.size()};
        const std::string* v[2] = {&a, &b};
        for (unsigned i = 0; i < n_; ++i) {
            bind[i].buffer_type = MYSQL_TYPE_STRING;
            bind[i].buffer = (void*)v[i]->data();
            bind[i].buffer_length = len[i];
            bind[i].length = &len[i];
        }
        if (mysql_stmt_bind_param(s_, bind) != 0 || mysql_stmt_execute(s_) != 0) {
            erro_ = mysql_stmt_error(s_);
            return false;
        }
        return true;
    }

private:
    MYSQL_STMT* s_ = nullptr;
    unsigned n_;
    std::string erro_;
};

class MariaDBStorage : public Storage {
public:
    explicit MariaDBStorage(Config cfg) : cfg_(std::move(cfg)) {}
    std::string nome() const override { return "MariaDB"; }

    bool salvar(const std::vector<Consulta>& pilha,
                const std::vector<Consulta>& fila, std::string& erro) override {
        Conexao db;
        if (!db.abrir(cfg_, erro)) return false;

        // Tudo em uma transação: se algo falhar, o banco continua como estava.
        if (!db.exec("START TRANSACTION", erro)) return false;
        bool ok = gravar(db, pilha, fila, erro);
        std::string erro2;
        if (ok) ok = db.exec("COMMIT", erro);
        else    db.exec("ROLLBACK", erro2);
        return ok;
    }

    bool carregar(std::vector<Consulta>& pilha,
                  std::vector<Consulta>& fila, std::string& erro) override {
        Conexao db;
        if (!db.abrir(cfg_, erro)) return false;
        pilha.clear(); fila.clear();
        // Mesma ordem em que foram salvas (topo -> base; início -> fim).
        return ler(db, "pilha_consultas", pilha, erro) && ler(db, "fila_atendimento", fila, erro);
    }

private:
    static bool gravar(Conexao& db, const std::vector<Consulta>& pilha,
                       const std::vector<Consulta>& fila, std::string& erro) {
        if (!db.exec("DELETE FROM pilha_consultas", erro)) return false;
        if (!db.exec("DELETE FROM fila_atendimento", erro)) return false;

        Stmt upsert(db.raw(), "INSERT INTO pacientes (id, nome) VALUES (?, ?) "
                              "ON DUPLICATE KEY UPDATE nome = VALUES(nome)", 2);
        Stmt inPilha(db.raw(), "INSERT INTO pilha_consultas (paciente_id) VALUES (?)", 1);
        Stmt inFila(db.raw(),  "INSERT INTO fila_atendimento (paciente_id) VALUES (?)", 1);
        for (Stmt* s : {&upsert, &inPilha, &inFila})
            if (!s->ok()) { erro = s->erro(); return false; }

        auto inserir = [&](const std::vector<Consulta>& lista, Stmt& destino) {
            for (const auto& c : lista) {
                if (!upsert.executar(c.id, c.nome)) { erro = upsert.erro(); return false; }
                if (!destino.executar(c.id))        { erro = destino.erro(); return false; }
            }
            return true;
        };
        return inserir(pilha, inPilha) && inserir(fila, inFila);
    }

    static bool ler(Conexao& db, const char* tabela, std::vector<Consulta>& saida, std::string& erro) {
        std::string sql = std::string("SELECT p.id, p.nome FROM ") + tabela +
                          " t JOIN pacientes p ON t.paciente_id = p.id ORDER BY t.id_registro ASC";
        if (!db.exec(sql, erro)) return false;
        MYSQL_RES* res = mysql_store_result(db.raw());
        if (!res) { erro = mysql_error(db.raw()); return false; }
        while (MYSQL_ROW row = mysql_fetch_row(res))
            saida.push_back({row[0] ? row[0] : "", row[1] ? row[1] : ""});
        mysql_free_result(res);
        return true;
    }

    Config cfg_;
};

} // namespace

std::unique_ptr<Storage> criarStorageMariaDB(const Config& cfg) {
    return std::make_unique<MariaDBStorage>(cfg);
}
bool temSuporteMariaDB() { return true; }

#else  // sem suporte

std::unique_ptr<Storage> criarStorageMariaDB(const Config&) { return nullptr; }
bool temSuporteMariaDB() { return false; }

#endif
