# 🏥 Sistema de Gerenciamento de Consultas e Atendimento em Clínica

Aplicação de terminal em **C++17**, em um único arquivo e usando só a biblioteca padrão, que
gerencia as consultas e o atendimento de uma clínica com três estruturas de dados
implementadas do zero:

- **Consultas agendadas**: lista duplamente encadeada, sempre ordenada por horário.
- **Fila de espera**: fila (FIFO) de pacientes aguardando atendimento.
- **Histórico de operações**: pilha (LIFO) com tudo o que foi feito na sessão.

Os dados de consultas e da fila podem ser salvos e carregados de um arquivo **CSV**.
Não há banco de dados nem dependências externas.

```
=======================================
     SISTEMA DE GESTAO DE CLINICA
=======================================
  1. Agendar consulta
  2. Cancelar consulta
  3. Mostrar consultas agendadas
  4. Adicionar paciente a fila de espera
  5. Chamar proximo paciente (atender)
  6. Mostrar fila de espera
  7. Gerar relatorio
  8. Mostrar historico de operacoes
  9. SALVAR dados em arquivo CSV
 10. CARREGAR dados de arquivo CSV
  0. Sair
---------------------------------------
  Consultas: 2 | Atendimentos: 1
---------------------------------------
  Escolha uma opcao:
```

## Índice

- [Recursos](#recursos)
- [Requisitos](#requisitos)
- [Compilação e execução](#compilação-e-execução)
- [Como usar](#como-usar)
- [Arquivo CSV](#arquivo-csv)
- [Estrutura do projeto](#estrutura-do-projeto)
- [Estruturas de dados](#estruturas-de-dados)
- [Limitações conhecidas](#limitações-conhecidas)
- [Solução de problemas](#solução-de-problemas)

## Recursos

- **Agendar consulta** informando paciente, médico e horário (`HH:MM`); a consulta já entra
  na posição certa da lista, ordenada por horário.
- **Cancelar consulta** pelo nome do paciente.
- **Fila de espera**: adicionar paciente, chamar o próximo (por ordem de chegada) e listar a fila.
- **Histórico de operações** em pilha, do registro mais recente para o mais antigo.
- **Relatório** com o total de consultas agendadas e de atendimentos realizados.
- **Salvar e carregar** consultas e fila em um arquivo CSV.
- Nomes com **espaços** (ex.: `Maria Souza`) funcionam normalmente.
- Toda a memória alocada é liberada ao sair do programa.

## Requisitos

- Um compilador com suporte a **C++17**: GCC (`g++`), Clang ou MSVC.

Nenhuma biblioteca adicional é necessária.

## Compilação e execução

### Linux e macOS

```bash
g++ -std=c++17 -Wall -Wextra -o clinica src/main.cpp
./clinica
```

Para instalar o compilador, caso ainda não tenha:

```bash
# Debian / Ubuntu / Mint
sudo apt install build-essential

# Fedora
sudo dnf install gcc-c++

# Arch / Manjaro
sudo pacman -S base-devel

# macOS
xcode-select --install
```

### Windows

Com **MinGW-w64 / MSYS2** (`g++` no `PATH`):

```bat
g++ -std=c++17 -Wall -Wextra -o clinica.exe src\main.cpp
clinica.exe
```

Com **Visual Studio** (*Developer Command Prompt*):

```bat
cl /EHsc /std:c++17 /utf-8 /Fe:clinica.exe src\main.cpp
clinica.exe
```

> **Atenção:** `CMakeLists.txt`, `build.sh` e `build.bat` ainda apontam para arquivos de uma
> versão anterior do projeto (`menu.cpp`, `storage_*.cpp` etc.), que não existem mais, e por isso
> **não compilam** no estado atual. Use os comandos acima.

## Como usar

Digite o número da opção e pressione **Enter**. Os campos de texto também são confirmados com Enter.

| Opção | Ação |
|---|---|
| `1` | Agendar consulta (pede paciente, médico e horário) |
| `2` | Cancelar consulta (pede o nome do paciente) |
| `3` | Mostrar consultas agendadas, em ordem de horário |
| `4` | Adicionar paciente ao final da fila de espera |
| `5` | Chamar o próximo paciente da fila (atender) |
| `6` | Mostrar a fila de espera |
| `7` | Gerar relatório |
| `8` | Mostrar o histórico de operações |
| `9` | Salvar dados no CSV |
| `10` | Carregar dados do CSV |
| `0` | Sair |

A barra de status mostra o número de consultas agendadas e de atendimentos feitos na sessão.

Exemplo de agendamento:

```
  Escolha uma opcao: 1
  Nome do paciente: Maria Souza
  Nome do medico:   Ana Lima
  Horario (HH:MM):  14:30
  Consulta agendada com sucesso!
```

Fluxo típico: **agendar** consultas, **adicionar** pacientes à fila e **chamar** o próximo conforme
o atendimento avança. Antes de sair, use a opção **9** para salvar; em outra sessão, a opção **10**
restaura os dados.

## Arquivo CSV

Os dados são gravados em `clinica_dados.csv`, na pasta de onde o programa foi executado. Cada linha
começa com um prefixo que indica o tipo do registro:

| Prefixo | Registro | Colunas |
|---|---|---|
| `C` | Consulta | `C,paciente,médico,horário` |
| `F` | Paciente na fila | `F,nome` |

```csv
C,Joao Silva,Carlos Dias,09:00
C,Maria Souza,Ana Lima,14:30
F,Luiza Reis
```

Ao **salvar**, o arquivo é sobrescrito. Ao **carregar**, as consultas e a fila em memória são
substituídas pelo conteúdo do arquivo e o contador de atendimentos volta a zero.

## Estrutura do projeto

```
.
├── src/
│   └── main.cpp        # todo o programa: estruturas, menu e persistência em CSV
├── CMakeLists.txt      # desatualizado (veja o aviso em "Compilação e execução")
├── build.sh            # desatualizado
├── build.bat           # desatualizado
└── .gitignore
```

O código de `src/main.cpp` é organizado em seções: nós das estruturas, variáveis globais (pontas
das estruturas), funções auxiliares, consultas, fila, histórico, CSV e menu principal.

## Estruturas de dados

| Estrutura | Implementação | Uso no sistema |
|---|---|---|
| Lista duplamente encadeada (`inicio` / `fim`) | Nós `Consulta` com ponteiros `anterior` e `proximo` | Consultas agendadas, inseridas já ordenadas por horário |
| Fila FIFO (`frente` / `tras`) | Nós `Paciente` com ponteiro `proximo` | Pacientes aguardando atendimento |
| Pilha LIFO (`topo`) | Nós `Operacao` com ponteiro `proximo` | Histórico de operações da sessão |

| Operação | Complexidade |
|---|---|
| Fila: enfileirar (`tras`) e chamar o próximo (`frente`) | O(1) |
| Pilha: registrar uma operação (`topo`) | O(1) |
| Consultas: agendar (busca da posição pelo horário) | O(n) |
| Consultas: cancelar (busca pelo nome do paciente) | O(n) |
| Listar consultas, fila ou histórico | O(n) |

A memória é alocada com `new` e liberada com `delete`: ao cancelar, ao chamar o próximo da fila,
ao carregar um CSV e ao encerrar o programa.

## Limitações conhecidas

- **Horário no formato `HH:MM` com dois dígitos** (`09:00`, não `9:00`): a ordenação compara texto,
  então `9:00` seria colocado depois de `14:30`. O programa não valida o horário nem rejeita
  campos vazios.
- **Não use vírgula em nomes.** O CSV não faz escape, então um nome como `Silva, Joao` é gravado
  e lido de forma incorreta ao carregar.
- **Cancelar** exige o nome do paciente **exatamente igual** ao cadastrado (diferencia maiúsculas
  de minúsculas) e remove apenas a primeira consulta encontrada.
- O **histórico de operações não é salvo** no CSV; ele existe apenas durante a sessão.
- A opção **0** sai **sem pedir confirmação** nem salvar os dados. O mesmo acontece se for digitado
  algo que não seja número no menu (uma letra, por exemplo): a entrada inválida é lida como `0` e o
  programa encerra.

## Solução de problemas

| Problema | Solução |
|---|---|
| `g++: command not found` | Instale um compilador C++ (veja [Compilação e execução](#compilação-e-execução)). |
| `Arquivo 'clinica_dados.csv' nao encontrado ou vazio` | Salve primeiro com a opção `9`, e execute o programa na mesma pasta onde o arquivo foi gravado. |
| `Cannot find source file: src/menu.cpp` ao usar CMake ou `build.sh` | Esses arquivos de build estão desatualizados; compile com `g++` conforme descrito acima. |
