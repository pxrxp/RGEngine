#!/bin/bash
set -e

# ----------------------------
# Usage:
#   TRIPLET=x64-mingw-dynamic ./build.sh [-c]
# ----------------------------

# Save current dir, move to script dir
ORIGINAL_DIR="$(pwd)"
cd "$(dirname "${BASH_SOURCE[0]}")"

CLEAN=false
while getopts "c" opt; do
    case $opt in
        c) CLEAN=true ;;
    esac
done

# Require TRIPLET
if [ -z "$TRIPLET" ]; then
    echo "Error: TRIPLET environment variable not set."
    echo "Example: TRIPLET=x64-mingw-dynamic ./build.sh"
    exit 1
fi

BUILD_TYPE=${BUILD_TYPE:-Release}
echo "Building for $BUILD_TYPE..."

# Bootstrap vcpkg if missing
if [ ! -d "./vcpkg" ]; then
    git clone https://github.com/microsoft/vcpkg.git vcpkg
    ./vcpkg/bootstrap-vcpkg.sh
fi

BUILD_DIR="build"
SRC_DIR="src"

# Clean only CMake cache/files if requested
if [ "$CLEAN" = true ]; then
    echo "Cleaning CMake cache..."
    [ -f "$BUILD_DIR/CMakeCache.txt" ] && rm "$BUILD_DIR/CMakeCache.txt"
    [ -d "$BUILD_DIR/CMakeFiles" ] && rm -rf "$BUILD_DIR/CMakeFiles"
fi

# Configure if missing
if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
    echo "Configuring CMake..."
    cmake -B "$BUILD_DIR" -S . \
        -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake \
        -DVCPKG_TARGET_TRIPLET="$TRIPLET" \
        -DVCPKG_HOST_TRIPLET="$TRIPLET" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

echo "Building..."
cmake --build "$BUILD_DIR" --parallel "$(nproc)"
echo "Copying shaders..."
[ -d "$BUILD_DIR/shaders" ] && rm -rf "$BUILD_DIR/shaders"
cp "$SRC_DIR/shaders" "$BUILD_DIR/shaders" -r

cd "$ORIGINAL_DIR"
