# 🏥 Sistema de Gerenciamento de Consultas e Atendimento em Clínica

Aplicação de terminal em **C++17** que gerencia uma **Pilha de Consultas (LIFO)** e uma
**Fila de Atendimento (FIFO)**, implementadas do zero com listas simplesmente encadeadas.
Os dados podem ser guardados em **MariaDB/MySQL** ou em um **arquivo CSV**.

Funciona em **Linux, macOS e Windows**, com o mesmo menu interativo navegável por setas.

```
=======================================
       SISTEMA DE GESTÃO CLÍNICA
=======================================
Use as setas (▲/▼) para navegar e Enter para confirmar:

  ➔  1. Adicionar Consulta (Pilha)
     2. Remover Consulta (Pilha)
     3. Mostrar Consultas
     4. Carregar Dados do MariaDB
     5. Salvar Dados no MariaDB
     6. Gerenciar Pilha
     7. Gerenciar Fila
     8. Sair do Programa

=======================================
Pilha: 2  |  Fila: 1  |  Dados: MariaDB  [não salvo]
```

## Índice

- [Recursos](#recursos)
- [Requisitos](#requisitos)
- [Instalação e compilação](#instalação-e-compilação)
  - [Linux](#linux)
  - [macOS](#macos)
  - [Windows](#windows)
- [Configuração](#configuração)
- [Banco de dados (MariaDB)](#banco-de-dados-mariadb)
- [Como usar](#como-usar)
- [Estrutura do projeto](#estrutura-do-projeto)
- [Estruturas de dados e eficiência](#estruturas-de-dados-e-eficiência)
- [Solução de problemas](#solução-de-problemas)

## Recursos

- Menu interativo com **setas ▲/▼**, **Enter**, atalhos numéricos (**1–8**) e **Esc** para voltar.
- **Pilha** de consultas: empilhar, desempilhar, ver topo, esvaziar.
- **Fila** de atendimento: adicionar paciente, chamar o próximo, ver o próximo, listar, esvaziar.
- **Dois modos de armazenamento**: MariaDB/MySQL ou CSV (escolhido automaticamente conforme o build).
- Nomes com **espaços e acentos** (ex.: `João da Silva`).
- Indicador na barra de status de **alterações não salvas** e confirmação ao sair.
- Configuração por **arquivo** ou **variáveis de ambiente** (sem senha escrita no código).

## Requisitos

| Item | Versão |
|---|---|
| Compilador C++17 | GCC 8+, Clang 7+, MSVC 2019+ ou MinGW-w64 |
| [CMake](https://cmake.org/download/) | 3.14+ |
| Cliente MariaDB/MySQL *(opcional)* | MariaDB Connector/C ou libmysqlclient |
| Servidor MariaDB/MySQL *(opcional)* | qualquer versão recente |

> O cliente MariaDB é **opcional**. Sem ele o programa compila normalmente e usa o modo CSV.

## Instalação e compilação

### Linux

Instale as dependências:

```bash
# Debian / Ubuntu / Mint
sudo apt install build-essential cmake libmariadb-dev

# Fedora
sudo dnf install gcc-c++ cmake mariadb-connector-c-devel

# Arch / Manjaro
sudo pacman -S base-devel cmake mariadb-libs
```

Compile e execute:

```bash
./build.sh
./build/clinica
```

Opções do `build.sh`: `--sem-mariadb` (só CSV) e `--debug`.

### macOS

```bash
brew install cmake mariadb-connector-c
cmake -S . -B build -DMARIADB_DIR="$(brew --prefix mariadb-connector-c)"
cmake --build build
./build/clinica
```

### Windows

Escolha **uma** das opções de compilador:

- **Visual Studio 2019+** (ou *Build Tools*), com a carga de trabalho *Desenvolvimento para desktop com C++*; ou
- **MSYS2 / MinGW-w64**: `pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake`.

Para o suporte a **MariaDB**, instale o
[MariaDB Connector/C](https://mariadb.com/downloads/connectors/) (64 bits) ou use o vcpkg
(`vcpkg install libmariadb:x64-windows`). O CMake procura em `C:\Program Files\MariaDB\...`
automaticamente; se estiver em outro lugar, informe com `-DMARIADB_DIR="C:/caminho"`.

Compile abrindo o **Developer Command Prompt** (Visual Studio) ou um terminal com `g++` no `PATH`:

```bat
build.bat
```

Para compilar **sem** MariaDB (apenas CSV): `build.bat sem-mariadb`.

Execute:

```bat
build\Release\clinica.exe     :: Visual Studio
build\clinica.exe             :: MinGW
```

> Com o MariaDB Connector/C, copie `libmariadb.dll` (pasta `lib` do Connector) para ao lado do `clinica.exe`.
>
> Use o **Windows Terminal** ou o PowerShell no Windows 10/11 para o melhor visual. Em consoles
> antigos, se os símbolos `➔ ▲ ▼` aparecerem quebrados, rode com `--ascii`.

## Configuração

Valores padrão < arquivo `clinica.conf` < variáveis de ambiente. Para começar, copie o modelo:

```bash
cp config/clinica.conf.example clinica.conf      # Windows: copy config\clinica.conf.example clinica.conf
```

| Chave (`clinica.conf`) | Variável de ambiente | Padrão | Descrição |
|---|---|---|---|
| `storage` | `CLINICA_STORAGE` | automático | `mariadb` ou `csv` |
| `db_host` | `CLINICA_DB_HOST` | `127.0.0.1` | Servidor do banco |
| `db_port` | `CLINICA_DB_PORT` | `3306` | Porta |
| `db_user` | `CLINICA_DB_USER` | `root` | Usuário |
| `db_pass` | `CLINICA_DB_PASS` | *(vazio)* | Senha |
| `db_name` | `CLINICA_DB_NAME` | `Sistema_de_Gerenciamento_de_consultas` | Nome do banco |
| `csv_path` | `CLINICA_CSV` | `clinica_dados.csv` | Arquivo do modo CSV |
| `ascii` | `CLINICA_ASCII` | `false` | Menu apenas com ASCII |

Exemplo rápido por variável de ambiente:

```bash
CLINICA_DB_HOST=100.120.87.102 CLINICA_DB_PASS=minha_senha ./build/clinica    # Linux/macOS
```
```powershell
$env:CLINICA_DB_PASS="minha_senha"; .\build\clinica.exe                        # PowerShell
```

**Linha de comando:** `--csv`, `--mariadb`, `--ascii`, `--versao`, `--ajuda`.

> 🔒 Não faça commit do `clinica.conf` com a senha real (ele já está no `.gitignore`).

## Banco de dados (MariaDB)

**Opção 1 – Docker ou Podman** (já configurado neste repositório):

```bash
docker compose up -d          # ou: podman-compose up -d
export CLINICA_DB_PASS=clinica123
./build/clinica
```

**Opção 2 – servidor já instalado.** O programa cria o banco e as tabelas sozinho ao
salvar pela primeira vez. Se preferir criar manualmente:

```bash
mariadb -u root -p < sql/schema.sql
```

| Tabela | Conteúdo |
|---|---|
| `pacientes` | Cadastro mestre (`id`, `nome`) |
| `pilha_consultas` | Registros da pilha, do topo à base (ordem por `id_registro`) |
| `fila_atendimento` | Registros da fila, na ordem de chamada |

Salvar é **transacional**: se algo falhar no meio, o banco continua exatamente como estava.

## Como usar

| Tecla | Ação |
|---|---|
| `▲` / `▼` | Navegar no menu |
| `Enter` | Confirmar |
| `1`–`9` | Escolher a opção direto |
| `Esc` | Voltar (nos submenus) |

Fluxo típico: **Adicionar Consulta** → **Gerenciar Fila** para enfileirar pacientes →
**Salvar Dados**. Em outra sessão, **Carregar Dados** restaura pilha e fila na ordem correta.

Regras de cadastro: o **ID** tem até 20 caracteres, sem espaços; o **nome**, até 100 caracteres.
Um mesmo ID não pode se repetir na mesma estrutura nem estar associado a nomes diferentes.

Formato do CSV (modo sem banco):

```csv
estrutura,id,nome
pilha,002,Maria Souza
pilha,001,João da Silva
fila,010,Ana Paula
```

## Estrutura do projeto

```
.
├── CMakeLists.txt            # build multiplataforma
├── build.sh / build.bat      # atalhos de compilação (Linux·macOS / Windows)
├── docker-compose.yml        # MariaDB pronto (Docker/Podman)
├── config/clinica.conf.example
├── sql/schema.sql            # esquema do banco
└── src/
    ├── main.cpp              # argumentos e escolha do armazenamento
    ├── menu.h / menu.cpp     # interface e regras do menu
    ├── estruturas.h          # Pilha e Fila (listas encadeadas)
    ├── terminal.h / .cpp     # teclado e cores: Linux/macOS (termios) e Windows (conio)
    ├── config.h / .cpp       # clinica.conf + variáveis de ambiente
    ├── storage.h             # interface de persistência
    ├── storage_mariadb.cpp   # MariaDB (statements preparados + transação)
    └── storage_csv.cpp       # CSV (gravação atômica)
```

## Estruturas de dados e eficiência

| Operação | Complexidade |
|---|---|
| Pilha: `push`, `pop`, `top` | O(1) |
| Fila: `push_back` (enfileirar) | O(1) — mantém ponteiro para o último nó |
| Fila: `pop_front`, `front` | O(1) |
| `size()` das duas estruturas | O(1) |

Detalhes de implementação:

- Memória gerenciada com `std::unique_ptr` (sem `new`/`delete` manual).
- A liberação das listas é **iterativa**, evitando estouro de pilha com listas enormes
  (testado com 1 milhão de registros carregados e liberados em menos de 1 s).
- No banco, são usados **statements preparados** (protege contra SQL injection) e
  **transações**; no CSV, a gravação usa arquivo temporário + renomeação.

## Solução de problemas

| Problema | Solução |
|---|---|
| `Cliente MariaDB não encontrado` no CMake | Instale o pacote de desenvolvimento (veja [Instalação](#instalação-e-compilação)) ou use `--sem-mariadb`. O programa continuará funcionando em modo CSV. |
| `Can't connect to server on '...'` | Confirme `db_host`, `db_port` e se o servidor está ativo (`docker compose ps`). |
| `Access denied for user` | Revise `db_user` / `db_pass`. |
| Aparecem códigos como `←[36m` no Windows | Use o Windows Terminal/PowerShell atual. O programa já ativa as cores ANSI do console. |
| Acentos ou símbolos quebrados no Windows | Use o Windows Terminal com fonte *Cascadia Mono* ou *Consolas*, ou rode com `--ascii`. |
| `libmariadb.dll` não encontrada (Windows) | Copie a DLL do MariaDB Connector/C para a pasta do `clinica.exe`. |
