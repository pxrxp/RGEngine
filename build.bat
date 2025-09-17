@echo off
IF NOT EXIST vcpkg (
    git clone https://github.com/microsoft/vcpkg.git vcpkg
    call vcpkg\bootstrap-vcpkg.bat
)

IF EXIST build\CMakeCache.txt del build\CMakeCache.txt
IF EXIST build\CMakeFiles rmdir /s /q build\CMakeFiles

cmake --preset vcpkg-release
cmake --build build --preset build-release
