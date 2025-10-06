cmake_minimum_required(VERSION 3.15)
project(RivalGrounds LANGUAGES CXX)

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
    set(VCPKG_APPLOCAL_DEPS "windows")
elseif (UNIX)
    option(USE_WAYLAND "Use Wayland backend for GLFW" ON)
    option(USE_X11 "Use X11 backend for GLFW" OFF)

    if (USE_WAYLAND)
        list(APPEND VCPKG_APPLOCAL_DEPS "wayland")
    elseif (USE_X11)
        list(APPEND VCPKG_APPLOCAL_DEPS "x11")
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

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(RivalGrounds ${SRC_FILES})

target_compile_options(RivalGrounds PRIVATE ${PROJECT_WARNING_FLAGS})

target_link_libraries(RivalGrounds PRIVATE
    glad::glad
    glfw
    imgui::imgui
    OpenGL::GL
    ZLIB::ZLIB
    PNG::PNG
)
