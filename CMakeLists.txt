cmake_minimum_required(VERSION 3.16.0)
project(utest VERSION 0.0.9)

set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_SOURCE_DIR}/ext/fetch" CACHE STRING "")
set(CPM_USE_LOCAL_PACKAGES ON CACHE BOOL "")
list(APPEND
    CMAKE_MODULE_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies
)

add_library(utest)
add_library(utest::utest ALIAS utest)
target_sources(utest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/utest.cc)
target_include_directories(utest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include(fmt)
target_link_libraries(utest PRIVATE fmt::fmt)

add_library(utest_main ${CMAKE_CURRENT_SOURCE_DIR}/utest_main.cc)
add_library(utest::main ALIAS utest_main)
target_link_libraries(utest_main PUBLIC utest::utest)
