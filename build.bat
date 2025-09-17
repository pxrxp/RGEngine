@echo off
setlocal

REM ----------------------------
REM Usage:
REM   set TRIPLET=x64-mingw-dynamic
REM   build.bat [-c]
REM ----------------------------

REM Save current directory
set "ORIGINAL_DIR=%CD%"

REM Move to the directory of this batch file
cd /d "%~dp0"

set CLEAN=false
if "%1"=="-c" set CLEAN=true

REM Require TRIPLET
if "%TRIPLET%"=="" (
    echo Error: TRIPLET environment variable not set.
    echo Example: set TRIPLET=x64-mingw-dynamic ^& build.bat
    exit /b 1
)

IF "%BUILD_TYPE%"=="" SET BUILD_TYPE=Release
echo Building for %BUILD_TYPE%...

REM Bootstrap vcpkg if missing
IF NOT EXIST ".\vcpkg" (
    git clone https://github.com/microsoft/vcpkg.git vcpkg
    call .\vcpkg\bootstrap-vcpkg.bat
)

SET BUILD_DIR=build
SET SRC_DIR=src
IF NOT EXIST "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Clean only CMake cache/files if requested
IF "%CLEAN%"=="true" (
    if exist "%BUILD_DIR%\CMakeCache.txt" del "%BUILD_DIR%\CMakeCache.txt"
    if exist "%BUILD_DIR%\CMakeFiles" rmdir /s /q "%BUILD_DIR%\CMakeFiles"
)

REM Configure if missing
IF NOT EXIST "%BUILD_DIR%\CMakeCache.txt" (
    echo Configuring CMake...
    cmake -B "%BUILD_DIR%" -S . ^
        -DCMAKE_TOOLCHAIN_FILE=.\vcpkg\scripts\buildsystems\vcpkg.cmake ^
        -DVCPKG_TARGET_TRIPLET=%TRIPLET% ^
        -DVCPKG_HOST_TRIPLET=%TRIPLET% ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
)

echo Building...
cmake --build "%BUILD_DIR%" --parallel %NUMBER_OF_PROCESSORS%

echo Copying shaders...
REM Remove the build shaders folder if it exists
if exist "%BUILD_DIR%\shaders" (
    rmdir /s /q "%BUILD_DIR%\shaders"
)

REM Copy shaders folder recursively
xcopy "%SRC_DIR%\shaders" "%BUILD_DIR%\shaders" /E /I /Y

cd /d "%ORIGINAL_DIR%"

endlocal

