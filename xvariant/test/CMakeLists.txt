cmake_minimum_required(VERSION 3.5)

set(MINIKANRENPP_XVARIANT_TEST "test_xvariant")

find_package(Catch2)
if(Catch2_FOUND)
    include(Catch)

    file(GLOB MINIKANRENPP_XVARIANT_TEST_SRCS "*.cpp")

    add_executable(${MINIKANRENPP_XVARIANT_TEST} ${MINIKANRENPP_XVARIANT_TEST_SRCS})
    target_link_libraries(${MINIKANRENPP_XVARIANT_TEST} ${MINIKANRENPP_XVARIANT} Catch2::Catch2)

    catch_discover_tests(${MINIKANRENPP_XVARIANT_TEST})
endif()
