#!/bin/bash

rm -rf build && mkdir build

cd build
cmake -DCMAKE_CXX_COMPILER=g++-13 ../../src
make