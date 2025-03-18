#!/bin/bash

set -e

clear 

rm -rf tmp-build 
mkdir tmp-build

cd tmp-build

cmake ..

echo "make -j$(nproc)"
make -j$(nproc)

./ex-rpc