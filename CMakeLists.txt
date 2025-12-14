cmake_minimum_required(VERSION 3.20)

project(csvd)

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