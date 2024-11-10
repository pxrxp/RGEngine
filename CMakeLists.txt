cmake_minimum_required(VERSION 3.10)
project(RivalGrounds)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GLAD_SOURCES_DIR "${PROJECT_SOURCE_DIR}/external/glad/")

find_package(PythonInterp REQUIRED)

file(GLOB_RECURSE PYTHON_SCRIPTS "src/*.py")
foreach(script ${PYTHON_SCRIPTS})
    execute_process(COMMAND ${PYTHON_EXECUTABLE} ${script})
endforeach()

add_subdirectory(external/glad/cmake)
add_subdirectory(external/glm)
add_subdirectory(external/glfw)

glad_add_library(glad STATIC API gl:core=4.6)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(RivalGrounds ${SOURCES})

target_include_directories(RivalGrounds PRIVATE
    external/glm
    external/glfw/include
    build/gladsources/glad/include

    src
)

target_link_libraries(RivalGrounds glad glm glfw)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(WIN32)
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/toolchains/win64.cmake")
    elseif(UNIX)
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/toolchains/linux.cmake")
    endif()
endif()

if(CMAKE_TOOLCHAIN_FILE MATCHES "win32")
    set(BUILD_DIR "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/win32")
elseif(CMAKE_TOOLCHAIN_FILE MATCHES "win64")
    set(BUILD_DIR "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/win64")
elseif(CMAKE_TOOLCHAIN_FILE MATCHES "linux")
    set(BUILD_DIR "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/linux")
else()
    message(FATAL_ERROR "Unsupported toolchain specified.")
endif()

add_custom_target(build ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${BUILD_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUILD_DIR}"
    COMMAND ${CMAKE_COMMAND} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ..
    COMMAND ${CMAKE_COMMAND} --build .
    WORKING_DIRECTORY "${BUILD_DIR}"
)

add_custom_target(build_all ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${BUILD_DIR}/win32"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUILD_DIR}/win32"
    COMMAND ${CMAKE_COMMAND} -DCMAKE_TOOLCHAIN_FILE="${CMAKE_CURRENT_SOURCE_DIR}/toolchains/win32.cmake" -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ..
    COMMAND ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE}
    
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${BUILD_DIR}/win64"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUILD_DIR}/win64"
    COMMAND ${CMAKE_COMMAND} -DCMAKE_TOOLCHAIN_FILE="${CMAKE_CURRENT_SOURCE_DIR}/toolchains/win64.cmake" -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ..
    COMMAND ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE}
    
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${BUILD_DIR}/linux"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUILD_DIR}/linux"
    COMMAND ${CMAKE_COMMAND} -DCMAKE_TOOLCHAIN_FILE="${CMAKE_CURRENT_SOURCE_DIR}/toolchains/linux.cmake" -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ..
    COMMAND ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE}
    
    WORKING_DIRECTORY ${BUILD_DIR}
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
