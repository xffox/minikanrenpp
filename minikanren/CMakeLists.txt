cmake_minimum_required(VERSION 3.5)

set(MINIKANREN "minikanren")
set(MINIKANREN ${MINIKANREN} PARENT_SCOPE)

set(MINIKANREN_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB SRCS "src/*.cpp" "src/**/*.cpp")

add_library(${MINIKANREN} ${SRCS})
target_include_directories(${MINIKANREN} PUBLIC "${MINIKANREN_INCLUDES}")
target_link_libraries(${MINIKANREN} PUBLIC ${XVARIANT})

add_subdirectory(test)
