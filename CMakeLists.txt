cmake_minimum_required(VERSION 3.22)

project(csvd)

include(cmake/CPM.cmake)

# Include expected implementation
# Reason: works on gcc and clang. The standard implementation is still lacking.
CPMAddPackage(
  NAME expected
  GITHUB_REPOSITORY TartanLlama/expected
  GIT_TAG v1.3.1
  OPTIONS
    "EXPECTED_BUILD_TESTS OFF"
)

add_library(${PROJECT_NAME} STATIC
    src/csvd.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    tl::expected
)

target_include_directories(${PROJECT_NAME} PUBLIC
    include
)

target_compile_features(${PROJECT_NAME} PUBLIC
    cxx_std_23
)

# build example

if(BUILD_EXAMPLE)
    add_executable(${PROJECT_NAME}_example
        examples/example.cpp
    )

    target_link_libraries(${PROJECT_NAME}_example
        ${PROJECT_NAME}
    )
endif()

option(${PROJECT_NAME}_enable_tests "Enables building test for csvd. Default: OFF" OFF)
if(${PROJECT_NAME}_enable_tests)
    message(STATUS "${PROJECT_NAME}: Building tests")

    CPMAddPackage("gh:google/googletest#v1.17.0")

    enable_testing()

    include(GoogleTest)

    add_executable(${PROJECT_NAME}_tests
        tests/tests.cpp
    )

    target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        ${PROJECT_NAME}
        GTest::gtest_main
    )

    # enable all errors on the test executable
    if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
        target_compile_options(${PROJECT_NAME}_tests PUBLIC
            -Wall
            -Wextra
            -Werror
        )
    endif()

    add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}_tests)

endif()