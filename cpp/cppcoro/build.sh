#!/bin/bash

# 编译协程示例
# 用法: ./build.sh [文件名，默认 coroutine_ucontext.cpp]

SRC="${1:-coroutine_ucontext.cpp}"
OUT="${SRC%.cpp}"

echo "编译: $SRC -> $OUT"

g++ -std=c++11 \
    -Wall -Wextra \
    -g \
    -O0 \
    -D_XOPEN_SOURCE=600 \
    "$SRC" -o "$OUT"

if [ $? -eq 0 ]; then
    echo "成功! 运行: ./$OUT"
else
    echo "编译失败"
    exit 1
fi
