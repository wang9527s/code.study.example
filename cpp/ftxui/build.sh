#!/bin/bash

set -e

if [ ! -d "./FTXUI" ]; then
    echo "download FTXUI"
    git clone https://github.com/ArthurSonzogni/FTXUI.git
    cd FTXUI/
    git checkout v4.1.1

    rm -rf build && mkdir build && cd build

    cmake .. -DBUILD_EXAMPLES=ON -DBUILD_TESTS=ON
    make -j4
fi

g++ demo.cpp -std=c++17 -pthread \
    -I ./FTXUI/include   \
    ./FTXUI/build/libftxui-component.a \
    ./FTXUI/build/libftxui-dom.a   \
    ./FTXUI/build/libftxui-screen.a   \
    -o demo