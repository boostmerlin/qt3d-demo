@echo off
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo reg dll requires administrative privileges.
    exit /b 1
) else (
    regsvr32 /s %~dp0msdia140.dll
)
