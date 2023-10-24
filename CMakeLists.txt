cmake_minimum_required(VERSION 3.16.0)
project(utest VERSION 0.0.1)

add_library(utest INTERFACE)
add_library(utest::utest ALIAS utest)
target_include_directories(utest INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(utest_main ${CMAKE_CURRENT_SOURCE_DIR}/utest_main.cc)
add_library(utest::main ALIAS utest_main)
target_link_libraries(utest_main PUBLIC utest::utest)
