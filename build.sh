#!/bin/bash
set -e

# ----------------------------
# Usage:
#   TRIPLET=x64-mingw-dynamic ./build.sh
# ----------------------------

# Require user to set TRIPLET
if [ -z "$TRIPLET" ]; then
    echo "Error: TRIPLET environment variable not set."
    echo "Example: TRIPLET=x64-mingw-dynamic ./build.sh"
    exit 1
fi

# Optional: choose build type
BUILD_TYPE=${BUILD_TYPE:-Release}

# Bootstrap vcpkg if missing
if [ ! -d "./vcpkg" ]; then
    git clone https://github.com/microsoft/vcpkg.git vcpkg
    ./vcpkg/bootstrap-vcpkg.sh
fi

# Run CMake configure using toolchain and triplet
cmake -B build -S . \
    -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET="$TRIPLET" \
    -DVCPKG_HOST_TRIPLET="$TRIPLET" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
# Build
cmake --build build
