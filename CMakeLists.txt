cmake_minimum_required(VERSION 3.15)
project(RivalGrounds LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)


file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(RivalGrounds ${SRC_FILES})

target_link_libraries(RivalGrounds PRIVATE
    glad::glad
    glfw
    imgui::imgui
    OpenGL::GL
    PNG::PNG
    ZLIB::ZLIB
)
