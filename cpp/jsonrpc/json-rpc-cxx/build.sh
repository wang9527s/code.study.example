#!/bin/bash

set -e

clear 

rm -rf tmp-build 
mkdir tmp-build

cd tmp-build

cmake -DCMAKE_BUILD_TYPE=Debug ..
# 默认 release
cmake ..

echo "make -j$(nproc)"
make -j$(nproc)

# gdb ./ex-rpc
./ex-rpc