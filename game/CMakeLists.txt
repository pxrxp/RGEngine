add_executable(RivalGrounds
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/RivalGroundsDebug.cpp
    ${CMAKE_SOURCE_DIR}/src/Enemy.cpp
)

target_compile_options(RivalGrounds PRIVATE ${PROJECT_WARNING_FLAGS})

target_link_libraries(RivalGrounds PRIVATE engine)
