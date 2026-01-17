@echo off
setlocal

set ROOT=%~dp0
set BUILD=%ROOT%build

if not exist "%BUILD%" mkdir "%BUILD%"

if not exist "%BUILD%\\sfml_chess.exe" (
  cmake -S "%ROOT%" -B "%BUILD%"
  cmake --build "%BUILD%" --config Release
)

set BIN=%BUILD%\\sfml_chess.exe
if not exist "%BIN%" set BIN=%BUILD%\\Release\\sfml_chess.exe
if not exist "%BIN%" set BIN=%BUILD%\\Debug\\sfml_chess.exe

if exist "%BIN%" (
  "%BIN%"
) else (
  echo Build finished but executable not found.
  exit /b 1
)

endlocal
