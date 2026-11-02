find_package(glm CONFIG REQUIRED)

add_executable(engine_tests
    test_main.cpp
    bounding_box_test.cpp
    triangle_mesh_collider_test.cpp
    # The tests only use the GL-free parts of the engine, so they compile the
    # translation units they need instead of linking the whole engine.
    ${CMAKE_SOURCE_DIR}/engine/src/components/BoundingBox.cpp
)

target_include_directories(engine_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/engine/include
)

target_compile_options(engine_tests PRIVATE ${PROJECT_WARNING_FLAGS})

target_link_libraries(engine_tests PRIVATE glm::glm)

add_test(NAME engine_tests COMMAND engine_tests)
