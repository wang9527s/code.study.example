#!/bin/bash

rm -rf build && mkdir build

cd build

cmake ../../example \
  -DCMAKE_CXX_COMPILER=g++-13 \
  -DCMAKE_BUILD_TYPE=Debug

make -j$(nproc)
