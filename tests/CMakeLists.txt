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
