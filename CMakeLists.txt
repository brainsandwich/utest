cmake_minimum_required(VERSION 3.16.0)
project(utest VERSION 0.0.1)

add_library(utest INTERFACE)
add_library(utest::utest ALIAS utest)
target_include_directories(utest INTERFACE ${PROJECT_SOURCE_DIR})

function(utest_create_suite TARGET)
    set(test_main ${PROJECT_SOURCE_DIR}/utest_main.cc)
    add_executable(${TARGET} ${test_main})
    target_link_library(${TARGET} utest::utest)
    unset(test_main)
endfunction()
