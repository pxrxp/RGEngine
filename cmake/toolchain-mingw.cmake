# Cross-compile for 64-bit Windows from Linux with llvm-mingw.
#
#   export LLVM_MINGW_ROOT=/path/to/llvm-mingw-<date>-ucrt-ubuntu-22.04-x86_64
#   cmake -B build-win -S . \
#     -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake" \
#     -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="$PWD/cmake/toolchain-mingw.cmake" \
#     -DVCPKG_TARGET_TRIPLET=x64-mingw-static \
#     -DVCPKG_HOST_TRIPLET=x64-linux \
#     -DCMAKE_BUILD_TYPE=Release -DENABLE_ASAN_UBSAN=OFF
#
# The find root matters: without it CMake happily links the host's libOpenGL.so
# into a Windows executable.

set(LLVM_MINGW_ROOT "$ENV{LLVM_MINGW_ROOT}" CACHE PATH "llvm-mingw installation directory")

if (NOT LLVM_MINGW_ROOT)
  message(FATAL_ERROR "LLVM_MINGW_ROOT is not set. Point it at an llvm-mingw directory (or set the environment variable).")
endif ()

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER "${LLVM_MINGW_ROOT}/bin/x86_64-w64-mingw32-clang")
set(CMAKE_CXX_COMPILER "${LLVM_MINGW_ROOT}/bin/x86_64-w64-mingw32-clang++")
set(CMAKE_RC_COMPILER "${LLVM_MINGW_ROOT}/bin/x86_64-w64-mingw32-windres")

set(CMAKE_FIND_ROOT_PATH "${LLVM_MINGW_ROOT}/x86_64-w64-mingw32")
# Packages and headers come from the vcpkg install tree, so search both the
# sysroot and the host prefixes; only programs must come from the host.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
