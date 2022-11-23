cmake_minimum_required(VERSION 3.5)

project(minikanren)

enable_testing()

set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -pedantic)

add_subdirectory(xvariant)
add_subdirectory(minikanren)
add_subdirectory(sample)
