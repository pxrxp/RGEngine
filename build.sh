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
        c ) CLEAN=true ;;
        \?) echo "Invalid option: -$OPTARG" >&2
            exit 1 ;;
    esac
done

# Require TRIPLET
if [ -z "$TRIPLET" ]; then
    echo "Error: TRIPLET environment variable not set."
    echo "Example: \"export TRIPLET=x64-mingw-dynamic\" (x64-mingw-dynamic|x64-linux|...)"
    echo "                       ^^^ (compulsorily no spaces)"
    exit 1
fi

# Require BUILD_TYPE
if [ -z "$BUILD_TYPE" ]; then
    echo "Error: BUILD_TYPE environment variable not set."
    echo "Example: \"export BUILD_TYPE=Debug\" (Debug|Release|...)"
    echo "                          ^^^ (compulsorily no spaces)"
    exit 1
fi

# Require ENABLE_ASAN_UBSAN
if [ -z "$ENABLE_ASAN_UBSAN" ]; then
    echo "Error: ENABLE_ASAN_UBSAN environment variable not set."
    echo "Example: \"export ENABLE_ASAN_UBSAN=OFF\" (OFF|ON)"
    echo "                                 ^^^ (compulsorily no spaces)"
    echo "(Enables Address Sanitizer, Undefined Behavior Sanitizer.)"
    echo "(May have issues with debugger if left ON.)"
    exit 1
fi

# Require ENABLE_WERROR
if [ -z "$ENABLE_WERROR" ]; then
    echo "Error: ENABLE_WERROR environment variable not set."
    echo "Example: \"export ENABLE_WERROR=OFF\" (OFF|ON)"
    echo "                             ^^^ (compulsorily no spaces)"
    echo "(Treats warnings as errors.)"
    exit 1
fi

echo "Building for $BUILD_TYPE..."

# Bootstrap vcpkg if missing
if [ ! -d "./vcpkg" ]; then
    git clone https://github.com/microsoft/vcpkg.git vcpkg
    ./vcpkg/bootstrap-vcpkg.sh
fi

BUILD_DIR="build"
SRC_DIR="src"
RESOURCES_DIR="resources"

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
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DENABLE_ASAN_UBSAN="$ENABLE_ASAN_UBSAN" \
        -DENABLE_WERROR="$ENABLE_WERROR" \
        -DVCPKG_TARGET_TRIPLET="$TRIPLET" \
        -DVCPKG_HOST_TRIPLET="$TRIPLET" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
fi

echo "Building..."
cmake --build "$BUILD_DIR" --parallel "$(nproc)"
echo "Copying shaders..."
[ -d "$BUILD_DIR/shaders" ] && rm -rf "$BUILD_DIR/shaders"
cp "$SRC_DIR/shaders" "$BUILD_DIR/shaders" -r
echo "Copying resources..."
[ -d "$BUILD_DIR/resources" ] && rm -rf "$BUILD_DIR/resources"
cp "$RESOURCES_DIR" "$BUILD_DIR/resources" -r

cd "$ORIGINAL_DIR"
