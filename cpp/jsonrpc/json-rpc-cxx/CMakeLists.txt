cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 17)
project(json-rpc-cxx VERSION 0.0.1 LANGUAGES CXX)


include(GNUInstallDirs)

add_library(json-rpc-cxx INTERFACE)
target_include_directories(json-rpc-cxx INTERFACE include)


find_package(Threads)
add_executable(ex-rpc
    examples/main/main.cpp
    examples/main/server.hpp
    examples/main/client.hpp
    examples/main/types.h
    examples/conn/inmemoryconnector.hpp
    examples/conn/cpphttplibconnector.hpp
)

target_compile_options(ex-rpc PUBLIC "${_warning_opts}")
target_link_libraries(ex-rpc json-rpc-cxx Threads::Threads)
target_include_directories(ex-rpc SYSTEM PRIVATE vendor)
target_include_directories(ex-rpc PRIVATE examples)
