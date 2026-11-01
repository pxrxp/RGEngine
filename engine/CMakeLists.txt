file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(engine STATIC ${ENGINE_SOURCES})

target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
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
