#!/usr/bin/env bash
# Compila o projeto no Linux/macOS:  ./build.sh [--sem-mariadb] [--debug]
set -euo pipefail
cd "$(dirname "$0")"

TIPO=Release; DB=ON
for a in "$@"; do
  case "$a" in
    --sem-mariadb) DB=OFF ;;
    --debug)       TIPO=Debug ;;
    -h|--help)     echo "Uso: ./build.sh [--sem-mariadb] [--debug]"; exit 0 ;;
    *) echo "Opção desconhecida: $a"; exit 1 ;;
  esac
done

command -v cmake >/dev/null || { echo "CMake não encontrado. Instale-o (veja o README.md)."; exit 1; }
cmake -S . -B build -DCMAKE_BUILD_TYPE="$TIPO" -DCLINICA_MARIADB="$DB"
cmake --build build --parallel
echo
echo "Pronto! Execute:  ./build/clinica"
