@echo off
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
