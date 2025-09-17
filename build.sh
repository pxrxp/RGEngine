#!/bin/bash
 
export VCPKG_DEFAULT_TRIPLET=x64-linux
export BUILD_TYPE=release

if [ ! -d "./vcpkg" ]; then
    git clone https://github.com/microsoft/vcpkg.git ./vcpkg
    ./vcpkg/bootstrap-vcpkg.sh
fi
[ -f build/CMakeCache.txt ] && rm build/CMakeCache.txt
[ -d build/CMakeFiles ] && rm -rf build/CMakeFiles

cmake --preset vcpkg-$BUILD_TYPE
cmake --build build --preset build-$BUILD_TYPE
