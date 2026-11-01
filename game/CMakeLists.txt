file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(RivalGrounds ${GAME_SOURCES})

target_include_directories(RivalGrounds
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(RivalGrounds PRIVATE ${PROJECT_WARNING_FLAGS})

target_link_libraries(RivalGrounds PRIVATE engine)
