file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

list(REMOVE_ITEM ENGINE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/RivalGroundsDebug.cpp
    ${CMAKE_SOURCE_DIR}/src/Enemy.cpp
)

add_library(engine STATIC ${ENGINE_SOURCES})

target_include_directories(engine
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_options(engine PRIVATE ${PROJECT_WARNING_FLAGS})

target_link_libraries(engine
    PUBLIC
        glad::glad
        glfw
        imgui::imgui
        OpenGL::GL
        ZLIB::ZLIB
        PNG::PNG
)
