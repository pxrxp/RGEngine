if [ ! -d "./vcpkg" ]; then
    git clone https://github.com/microsoft/vcpkg.git ./vcpkg
    ./vcpkg/bootstrap-vcpkg.sh
    ./vcpkg/vcpkg install
fi
rm -rf build
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
