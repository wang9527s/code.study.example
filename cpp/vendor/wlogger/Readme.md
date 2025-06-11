
### 前言

想找一个简单点的，日志库，最好是只有hpp文件的。  

发现之前用过的loguru里面宏太多。
spdlog就更复杂了，很多文件，一晚上没找到想要的。

最后发现[我的现代 C++ 日志系统实现心得](https://zhuanlan.zhihu.com/p/7580825580)，写的不多，作者的话符合我的想法。  
然后就有了造轮子的想法。

参考[blitz_logger](https://github.com/Pp3ng/blitz_logger )


### 编译

代码中使用了很多c++20新特性，    g++ 13 和 vs 2019 才支持。

+ **ubuntu 升级g++**

```bash
    sudo apt update
    sudo apt install -y software-properties-common
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    sudo apt update
    
    sudo apt install -y g++-13
```

**注意，是ubuntu，不是debian**

### 迭代

#### 2025.06.11

  我台式机上测试，大概的速度是 1,335,372.00 msgs/s