@echo off

if "%~2"=="" (
  echo Usage: %~nx0 ^<path to symbols dir^> ^<path to source sym file^>
  exit /b 1
)

set "SYMBOLS_DIR=%~1"
set "SOURCE_SYM=%~2"

if not exist "%SOURCE_SYM%" (
  echo Source sym file not found: %SOURCE_SYM%
  exit /b 1
)

if not exist "%SYMBOLS_DIR%" (
  mkdir "%SYMBOLS_DIR%"
)

for /f "tokens=4,5" %%a in ('type "%SOURCE_SYM%" ^| findstr /n "^" ^| findstr "^1:"') do (
  set "id=%%a"
  set "name=%%b"
)

if not exist "%SYMBOLS_DIR%\%name%\%id%" (
  mkdir "%SYMBOLS_DIR%\%name%\%id%"
)

move "%SOURCE_SYM%" "%SYMBOLS_DIR%\%name%\%id%\"