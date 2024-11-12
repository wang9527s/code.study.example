cmake_minimum_required(VERSION 3.12)
project(example)

set(CMAKE_CXX_STANDARD 11)

set(pybind11_DIR "$ENV{HOME}/.local/lib/python3.8/site-packages/pybind11/share/cmake/pybind11")

# 引入 pybind11
find_package(pybind11 REQUIRED)

file(GLOB SRCS 
    binding.cpp
	)

# 编译 C++ 文件
add_library(example MODULE ${SRCS})
target_link_libraries(example PRIVATE pybind11::module)
