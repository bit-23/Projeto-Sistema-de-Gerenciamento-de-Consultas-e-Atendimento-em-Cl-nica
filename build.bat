@echo off
rem Compila o projeto no Windows.  Uso: build.bat [sem-mariadb]
rem Funciona com Visual Studio (Developer Command Prompt) ou MinGW-w64 (g++ no PATH).
setlocal
cd /d "%~dp0"

set DB=ON
if /I "%1"=="sem-mariadb" set DB=OFF

where cmake >nul 2>nul
if errorlevel 1 (
  echo CMake nao encontrado. Instale-o em https://cmake.org/download/ ^(veja o README.md^).
  exit /b 1
)

where cl >nul 2>nul
if %errorlevel%==0 (
  cmake -S . -B build -DCLINICA_MARIADB=%DB%
  if errorlevel 1 exit /b 1
  cmake --build build --config Release
  set EXE=build\Release\clinica.exe
) else (
  where g++ >nul 2>nul
  if errorlevel 1 (
    echo Nenhum compilador encontrado. Instale o Visual Studio Build Tools ou o MinGW-w64.
    exit /b 1
  )
  cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCLINICA_MARIADB=%DB%
  if errorlevel 1 exit /b 1
  cmake --build build
  set EXE=build\clinica.exe
)
if errorlevel 1 exit /b 1
echo.
echo Pronto! Execute:  %EXE%
