cmake_minimum_required(VERSION 3.5)

project(minikanrenpp)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

enable_testing()

set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -pedantic)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/miniKanrenppConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/miniKanrenpp")

install(EXPORT miniKanrenppTargets NAMESPACE minikanrenpp::
    FILE "miniKanrenppConfig.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/miniKanrenpp")
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/miniKanrenppConfig.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/miniKanrenpp")

add_subdirectory(xvariant)
add_subdirectory(minikanren)
if(ENABLE_SAMPLE)
    add_subdirectory(sample)
endif()
