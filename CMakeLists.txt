cmake_minimum_required(VERSION 3.15)
project(RGEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_WERROR "Treat warnings as errors." ON)
option(ENABLE_ASAN_UBSAN "Enable Address and UndefinedBehavior Sanitizers (recommended for debug)." ON)
option(ENABLE_TSAN "Enable ThreadSanitizer (for multi-threaded code)." OFF)

set(PROJECT_WARNING_FLAGS
    -Wall
    -Wextra
    -pedantic
)

if (WIN32)
    # Copy the MinGW runtime next to the executable so a release archive runs
    # without a separate DLL install step. Cross builds have no Windows host to
    # run the app-local step, and the static triplet has no DLLs anyway.
    if (CMAKE_CROSSCOMPILING)
        set(VCPKG_APPLOCAL_DEPS OFF)
    else()
        set(VCPKG_APPLOCAL_DEPS ON)
    endif()
elseif (UNIX)
    option(USE_WAYLAND "Use Wayland backend for GLFW" ON)
    option(USE_X11 "Use X11 backend for GLFW" OFF)

    if (NOT DEFINED VCPKG_MANIFEST_FEATURES)
        if (USE_WAYLAND)
            set(VCPKG_MANIFEST_FEATURES "wayland")
        elseif (USE_X11)
            set(VCPKG_MANIFEST_FEATURES "x11")
        endif()
    endif()
endif()

if(ENABLE_ASAN_UBSAN)
    message(STATUS "AddressSanitizer and UndefinedBehaviorSanitizer enabled.")
    set(SANITIZER_FLAGS "-fsanitize=address,undefined")
elseif(ENABLE_TSAN)
    message(STATUS "ThreadSanitizer enabled.")
    set(SANITIZER_FLAGS "-fsanitize=thread")
endif()

if(SANITIZER_FLAGS)
    add_compile_options("-g" ${SANITIZER_FLAGS})
    add_link_options(${SANITIZER_FLAGS})
endif()

if(MINGW)
    # Link the MinGW C++ runtime and unwinder statically so a release archive
    # does not depend on libc++.dll / libgcc_s_seh-1.dll being installed.
    add_link_options(-static)
endif()

if(ENABLE_WERROR)
    list(APPEND PROJECT_WARNING_FLAGS -Werror)
    message(STATUS "Treating warnings as errors.")
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3")
    message(STATUS "Enabled rich debug symbols (-g3) for GDB macro expansion.")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

add_subdirectory(engine)
add_subdirectory(game)

enable_testing()
add_subdirectory(tests)

# --------------------------------------------------------------- releases --
# `cmake --build build --target package` produces a runnable archive with the
# shaders and assets the game resolves relative to the executable.
install(TARGETS RivalGrounds RUNTIME DESTINATION .)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/engine/shaders/ DESTINATION shaders)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/game/shaders/ DESTINATION shaders)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/ DESTINATION resources)
install(FILES ${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_SOURCE_DIR}/LICENSES.md
        DESTINATION .)

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "RGEngine-RivalGrounds")
set(CPACK_PACKAGE_VENDOR "RGEngine")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME
    "RGEngine-RivalGrounds-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "RGEngine and the RivalGrounds game, shaders and assets included")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)
