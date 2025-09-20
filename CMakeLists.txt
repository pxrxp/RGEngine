cmake_minimum_required(VERSION 3.15)
project(RivalGrounds LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

function(enable_preprocessing_on_build TARGET_NAME)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${TARGET_NAME} PRIVATE "-save-temps=obj")
        message(STATUS "Enabled preprocessor output (-save-temps) for target ${TARGET_NAME}")

    elseif(MSVC)
        set(PP_TARGET_NAME "preprocess_${TARGET_NAME}")
        get_target_property(SOURCES ${TARGET_NAME} SOURCES)
        get_target_property(INCLUDES ${TARGET_NAME} INCLUDE_DIRECTORIES)
        get_target_property(DEFINES ${TARGET_NAME} COMPILE_DEFINITIONS)
        get_target_property(COMPILE_OPTIONS ${TARGET_NAME} COMPILE_OPTIONS)

        set(ALL_PP_FILES "")

        foreach(SOURCE_FILE ${SOURCES})
            if(NOT IS_ABSOLUTE ${SOURCE_FILE})
                set(SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}")
            endif()

            get_filename_component(BASENAME ${SOURCE_FILE} NAME_WE)
            set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.i")
            list(APPEND ALL_PP_FILES ${OUTPUT_FILE})

            add_custom_command(
                OUTPUT ${OUTPUT_FILE}
                COMMAND ${CMAKE_CXX_COMPILER}
                        /I "${INCLUDES}"
                        /D "${DEFINES}"
                        ${COMPILE_OPTIONS}
                        /P
                        ${SOURCE_FILE}
                        /Fi"${OUTPUT_FILE}"
                DEPENDS ${SOURCE_FILE}
            )
        endforeach()

        add_custom_target(${PP_TARGET_NAME} ALL DEPENDS ${ALL_PP_FILES})
        message(STATUS "Enabled preprocessor output (custom target) for target ${TARGET_NAME}")
    else()
        message(WARNING "Preprocessing not enabled for ${TARGET_NAME}: Unsupported compiler ${CMAKE_CXX_COMPILER_ID}")
    endif()

endfunction()
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(RivalGrounds ${SRC_FILES})

target_link_libraries(RivalGrounds PRIVATE
    glad::glad
    glfw
    imgui::imgui
    OpenGL::GL
    ZLIB::ZLIB
    PNG::PNG
)

enable_preprocessing_on_build(RivalGrounds)
