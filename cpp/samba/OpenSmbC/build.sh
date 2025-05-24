#!/bin/bash

# sudo apt install make g++ libssl-dev libpopt-dev libkrb5-dev

set -e

rm -rf build && mkdir build
cd build
cmake ..
make -j$(nproc)
