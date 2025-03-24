#!/bin/bash

git status -s -uall | cut -c 4- | grep -E '*\.(cpp|h|hpp)$' | while read file; 
do 
    clang-format --style=file -i --verbose $file ;
done