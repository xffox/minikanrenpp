cmake_minimum_required(VERSION 3.5)

set(XVARIANT_TEST "test_xvariant")

find_package(Catch2)
if(Catch2_FOUND)
    include(Catch)

    file(GLOB XVARIANT_TEST_SRCS "*.cpp")

    add_executable(${XVARIANT_TEST} ${XVARIANT_TEST_SRCS})
    target_link_libraries(${XVARIANT_TEST} ${XVARIANT} Catch2::Catch2)

    catch_discover_tests(${XVARIANT_TEST})
endif()
