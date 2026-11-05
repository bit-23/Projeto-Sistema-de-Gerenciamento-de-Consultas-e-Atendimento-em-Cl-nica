cmake_minimum_required(VERSION 3.14)
project(clinica VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(CLINICA_MARIADB "Compilar com suporte a MariaDB/MySQL (se encontrado)" ON)

add_executable(clinica
  src/main.cpp
  src/menu.cpp
  src/terminal.cpp
  src/config.cpp
  src/storage_csv.cpp
  src/storage_mariadb.cpp
)
target_compile_definitions(clinica PRIVATE CLINICA_VERSAO="${PROJECT_VERSION}")

if(MSVC)
  target_compile_options(clinica PRIVATE /W4 /utf-8)
else()
  target_compile_options(clinica PRIVATE -Wall -Wextra)
endif()

# --- Cliente MariaDB/MySQL (opcional) ---------------------------------------
# Windows: instale o "MariaDB Connector/C" (ou use vcpkg: libmariadb) e, se
# necessário, informe o caminho com -DMARIADB_DIR="C:/Program Files/MariaDB/MariaDB Connector C 64-bit"
set(_HINTS "")
if(MARIADB_DIR)
  list(APPEND _HINTS "${MARIADB_DIR}")
elseif(DEFINED ENV{MARIADB_DIR})
  list(APPEND _HINTS "$ENV{MARIADB_DIR}")
endif()
if(WIN32)
  file(GLOB _WIN_DIRS "C:/Program Files/MariaDB/MariaDB Connector C*"
                      "C:/Program Files/MariaDB/*Connector*C*"
                      "C:/Program Files/MySQL/MySQL Server*"
                      "C:/msys64/mingw64" "C:/msys64/ucrt64")
  list(APPEND _HINTS ${_WIN_DIRS})
endif()

if(CLINICA_MARIADB)
  find_path(MARIADB_INCLUDE_DIR NAMES mysql.h
            HINTS ${_HINTS} PATH_SUFFIXES include include/mariadb include/mysql mariadb mysql)
  find_library(MARIADB_LIBRARY NAMES mariadb libmariadb mariadbclient mysqlclient libmysql
               HINTS ${_HINTS} PATH_SUFFIXES lib lib/mariadb lib64 lib/x86_64-linux-gnu)
  if(MARIADB_INCLUDE_DIR AND MARIADB_LIBRARY)
    message(STATUS "MariaDB: ${MARIADB_LIBRARY}")
    target_include_directories(clinica PRIVATE "${MARIADB_INCLUDE_DIR}")
    target_link_libraries(clinica PRIVATE "${MARIADB_LIBRARY}")
    target_compile_definitions(clinica PRIVATE HAVE_MARIADB)
  else()
    message(WARNING "Cliente MariaDB não encontrado: compilando só com suporte a CSV. "
                    "Veja a seção 'Instalação' do README.md.")
  endif()
endif()

install(TARGETS clinica RUNTIME DESTINATION bin)
install(FILES config/clinica.conf.example sql/schema.sql DESTINATION share/clinica)
