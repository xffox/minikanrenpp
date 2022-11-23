cmake_minimum_required(VERSION 3.5)

set(MINIKANREN_TEST "test_minikanren")

find_package(Catch2)
if(Catch2_FOUND)
    include(Catch)

    file(GLOB MINIKANREN_TEST_SRCS "*.cpp")

    add_executable(${MINIKANREN_TEST} ${MINIKANREN_TEST_SRCS})
    target_link_libraries(${MINIKANREN_TEST} ${MINIKANREN} Catch2::Catch2)

    catch_discover_tests(${MINIKANREN_TEST})
endif()
