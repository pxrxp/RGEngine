@echo off

IF %errorlevel% NEQ 0 (
    exit /b 1
)

REM Save current directory
set "ORIGINAL_DIR=%CD%"

REM Move to the directory of this batch file
cd /d "%~dp0"

set BINARY=build\RivalGrounds.exe

IF EXIST "%BINARY%" (
    echo Running %BINARY%...
    "%BINARY%"
) ELSE (
    echo Binary not found. Building first...
    call build.bat
    echo Running %BINARY%...
    "%BINARY%"
)

cd /d "%ORIGINAL_DIR%"
