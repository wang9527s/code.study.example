import sys
import os

# 将当前目录添加到 sys.path
sys.path.append(os.getcwd())

import example

def my_callback(value):
    print(f"Callback called with value: {value}")

cpp_obj = example.CallbackExample()
cpp_obj.setCallback(my_callback)
cpp_obj.triggerCallback(42)
