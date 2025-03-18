#!/bin/bash

# 删除aiso中没有使用到的 hpp 文件 

set -e

cd asio


# g++ -M -I. asio.hpp | grep -o '\S*asio\S*' | sort | uniq | sed 's|^\./||'
# g++ -M -I. asio.hpp | grep asio | sort  | cut -d ' ' -f 2
# exit

dependency_files=$(g++ -M -I. asio.hpp | grep -o '\S*asio\S*' | sort | uniq | sed 's|^\./||')
local_files=$(find . -type f | sed 's|^\./||')
for need in $dependency_files; do 
    echo "need " ${need}
done

for local_file in $local_files ; do
    if echo "$dependency_files" | grep -Fxq "$local_file"; then
        echo "ignore " ${local_file}
    else
        echo "rm: $local_file"
        rm "$local_file"
        # aaa=$local_file      
    fi
done