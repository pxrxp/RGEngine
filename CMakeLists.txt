cmake_minimum_required(VERSION 3.10)
project(RivalGrounds)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})

set(GLAD_SOURCES_DIR "${PROJECT_SOURCE_DIR}/external/glad/")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "MinSizeRel" CACHE STRING "Build type" FORCE)
endif()


find_package(Python REQUIRED COMPONENTS Interpreter)

file(GLOB_RECURSE PYTHON_SCRIPTS "src/*.py")
foreach(script ${PYTHON_SCRIPTS})
    execute_process(COMMAND ${Python_EXECUTABLE} ${script})
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

###################################################
if(CMAKE_BUILD_TYPE STREQUAL "Debug")

enable_testing()
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(external/googletest)

file(GLOB_RECURSE TESTFILES "tests/*.cc" "src/Core/*.cpp")
add_executable(UnitTests ${TESTFILES})

target_include_directories(UnitTests PRIVATE
    external/googletest/include
    external/glm
    external/glfw/include
    build/gladsources/glad/include

    src
    tests
)

target_link_libraries(UnitTests gtest_main glad glm glfw)
include(GoogleTest)
gtest_add_tests(UnitTests "" AUTO)

endif()
