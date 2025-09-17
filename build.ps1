if (-Not (Test-Path -Path ".\vcpkg")) {
    git clone https://github.com/microsoft/vcpkg.git .\vcpkg
    .\vcpkg\bootstrap-vcpkg.bat
}

if (Test-Path -Path ".\build\CMakeCache.txt") {
    Remove-Item ".\build\CMakeCache.txt"
}
if (Test-Path -Path ".\build\CMakeFiles") {
    Remove-Item ".\build\CMakeFiles" -Recurse -Force
}

cmake --preset vcpkg-release
cmake --build .\build --preset build-release
