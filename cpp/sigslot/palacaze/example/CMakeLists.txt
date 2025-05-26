cmake_minimum_required(VERSION 3.5)

file(GLOB_RECURSE UNIT_TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
foreach(ex IN LISTS UNIT_TESTS)
    string(REPLACE ".cpp" "" target ${ex})
    string(REGEX REPLACE "/" "." target ${target})
    set(target ex-${target})

    add_executable(${target} "${ex}")
endforeach()
