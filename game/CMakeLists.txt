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

# Assets are resolved relative to the executable, so stage them next to it.
add_custom_command(TARGET RivalGrounds POST_BUILD
    # Replace the staged shader directory so removed shaders do not linger.
    COMMAND ${CMAKE_COMMAND} -E rm -rf $<TARGET_FILE_DIR:RivalGrounds>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/engine/shaders $<TARGET_FILE_DIR:RivalGrounds>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/game/shaders $<TARGET_FILE_DIR:RivalGrounds>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:RivalGrounds>/resources
    COMMENT "Staging shaders and resources next to the executable"
)
