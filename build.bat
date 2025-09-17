@echo off
REM ----------------------------------------
REM Usage:
REM   set TRIPLET=x64-mingw-dynamic
REM   build.bat
REM ----------------------------------------

REM Check if TRIPLET is set
IF "%TRIPLET%"=="" (
    echo Error: TRIPLET environment variable not set.
    echo Example: set TRIPLET=x64-mingw-dynamic ^& build.bat
    exit /b 1
)

REM Optional: build type
IF "%BUILD_TYPE%"=="" SET BUILD_TYPE=Release

REM Bootstrap vcpkg if missing
IF NOT EXIST ".\vcpkg" (
    git clone https://github.com/microsoft/vcpkg.git vcpkg
    call .\vcpkg\bootstrap-vcpkg.bat
)

REM Create build directory if missing
IF NOT EXIST "build" mkdir build

REM Remove old cache to ensure toolchain is applied
IF EXIST build\CMakeCache.txt del /f build\CMakeCache.txt
IF EXIST build\CMakeFiles rd /s /q build\CMakeFiles

REM Configure CMake
cmake -B build -S . ^
    -DCMAKE_TOOLCHAIN_FILE=.\vcpkg\scripts\buildsystems\vcpkg.cmake ^
    -DVCPKG_TARGET_TRIPLET=%TRIPLET% ^
    -DVCPKG_HOST_TRIPLET=%TRIPLET% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

REM Build
cmake --build build
