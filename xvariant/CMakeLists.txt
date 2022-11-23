cmake_minimum_required(VERSION 3.5)

set(XVARIANT "xvariant")
set(XVARIANT ${XVARIANT} PARENT_SCOPE)

set(XVARIANT_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(${XVARIANT} INTERFACE)
target_include_directories(${XVARIANT} INTERFACE "${XVARIANT_INCLUDES}")

add_subdirectory(test)
