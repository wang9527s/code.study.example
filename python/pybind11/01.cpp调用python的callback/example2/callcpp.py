import sys
import os

# 将当前目录添加到 sys.path
# sys.path.append(os.getcwd())

import example

# 定义一个回调类
class MyCallbackHandler:
    def onCallback(self, value):
        print(f"Python: MyCallbackHandler received value: {value}")

# 创建 C++ 对象
cpp_obj = example.CallbackExample()

# 创建回调对象实例，并将其传递给 C++ 对象
callback_handler = MyCallbackHandler()
cpp_obj.setCallback(callback_handler)

# 触发回调
cpp_obj.triggerCallback(42)
