@echo off

set VCPKG_DEFAULT_TRIPLET=x64-mingw
set BUILD_TYPE=release

IF NOT EXIST vcpkg (
    git clone https://github.com/microsoft/vcpkg.git vcpkg
    call vcpkg\bootstrap-vcpkg.bat
)

IF EXIST build\CMakeCache.txt del build\CMakeCache.txt
IF EXIST build\CMakeFiles rmdir /s /q build\CMakeFiles

cmake --preset vcpkg-%BUILD_TYPE%
cmake --build build --preset build-%BUILD_TYPE%
