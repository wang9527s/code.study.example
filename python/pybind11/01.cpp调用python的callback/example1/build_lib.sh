#!/bin/bash

set -e

rm -rf build
mkdir build && cd build
cmake ..
make -j16
cp libexample.so ../example.so