cmake_minimum_required(VERSION 3.5)

set(MINIKANREN_SAMPLE "sample")

file(GLOB MINIKANREN_SAMPLE_SRCS "src/*")

add_executable(${MINIKANREN_SAMPLE} "${MINIKANREN_SAMPLE_SRCS}")
target_link_libraries(${MINIKANREN_SAMPLE} ${MINIKANREN})
