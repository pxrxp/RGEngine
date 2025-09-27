@echo off
setlocal

IF %errorlevel% NEQ 0 (
    exit /b 1
)

IF "%1"=="-d" set RENDERDOC=true
IF "%1"=="-g" set GDB=true

REM Require BUILD_TYPE
if "%BUILD_TYPE%"=="" (
    echo Error: BUILD_TYPE environment variable not set.
    echo Example: set BUILD_TYPE=Release ^| Debug
    exit /b 1
)

REM Save current directory
set "ORIGINAL_DIR=%CD%"

REM Move to the directory of this batch file
cd /d "%~dp0"

set BINARY="%ORIGINAL_DIR%\build\RivalGrounds.exe"

IF EXIST "%BINARY%" (
    IF "%RENDERDOC%"=="true" (
        echo Debugging %BINARY% using RenderDoc...
        start "" "qrenderdoc" "%BINARY%"
    ) ELSE IF "%GDB%"=="true" (
        echo Debugging %BINARY% using GDB...
        start "" /MAX "gdb" "%BINARY%"
    ) ELSE (
        echo Running %BINARY%...
        "%BINARY%"
    )
) ELSE (
    echo Binary not found. Building first...
    call build.bat
    echo Running %BINARY%...
    "%BINARY%"
)

cd /d "%ORIGINAL_DIR%"
endlocal
