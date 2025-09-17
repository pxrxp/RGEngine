if [ ! -d "./vcpkg" ]; then
    git clone https://github.com/microsoft/vcpkg.git ./vcpkg
    ./vcpkg/bootstrap-vcpkg.sh
fi
[ -f build/CMakeCache.txt ] && rm build/CMakeCache.txt
[ -d build/CMakeFiles ] && rm -rf build/CMakeFiles

cmake --preset vcpkg-release
cmake --build build --preset build-release
