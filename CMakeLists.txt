cmake_minimum_required(VERSION 3.22)

project(csvd)

include(cmake/BPM.cmake)

BPMAddInstallPackage("https://github.com/google/googletest#v1.17.0" PACKAGES GTest PRIVATE)

BPMMakeAvailable()


add_library(${PROJECT_NAME} STATIC
    src/csvd.cpp
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

option(CSVD_ENABLE_TESTING "Enables building test for csvd. Default: OFF" OFF)
if(CSVD_ENABLE_TESTING)
    message(STATUS "${PROJECT_NAME}: Building tests")

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