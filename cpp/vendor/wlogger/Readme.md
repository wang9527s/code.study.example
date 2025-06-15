
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

### 迭代 (单生产者，性能)

#### 2025.06.11

  我台式机上测试，大概的速度是 **1,335,372.00 msgs/s**

#### 2025.06.12

  Context中的filename、function等字符串改为 std::source_location，略有提升
  **1,638,476.00 msgs/s**

  封装一个格式化时间的类，速度 -> **2045840.00 msgs/s**

#### 2025.06.14

```
std::format_to(std::back_inserter(_fileBuffer), "{}\n", msg_format); 
```

 改为

```
_fileBuffer.insert(_fileBuffer.end(), msg_format.begin(), msg_format.end());
_fileBuffer.push_back('\n');
```

  速度 -> **2356789.00 msgs/s**

移除时间戳统计

  速度 -> **[2868657.00, 3175424.00] msgs/s**

#### 2025.06.15

使用更精确的统计方式（新能略微降低），速度 -> **[30,485,919.01,  3,187,827.31]msgs/s**  

### 多生产者

#### 初版

  多线程，性能下降严重

```
------------ producer Th count :1--------------
生产 : msg_count 10,000,000, total_ns 3046617623, 304.66 ns/msg, 3,282,328.55 msgs/s
消费 : msg_count 10,000,000, total_ns 3086170664, 308.62 ns/msg, 3,240,261.50 msgs/s
push_failed.  compare_exchange_weak failed: 0, buffer_is_full 206,660,317
pop_failed.  24,070,192

------------ producer Th count :5--------------
生产 : msg_count 10,000,000, total_ns 5324842343, 532.48 ns/msg, 1,877,989.87 msgs/s
消费 : msg_count 10,000,000, total_ns 5369487397, 536.95 ns/msg, 1,862,375.17 msgs/s
push_failed.  compare_exchange_weak failed: 5,141,291, buffer_is_full 377,058,090
pop_failed.  24,056,984

------------ producer Th count :8--------------
生产 : msg_count 10,000,000, total_ns 10528176995, 1052.82 ns/msg, 949,832.06 msgs/s
消费 : msg_count 10,000,000, total_ns 10582793967, 1058.28 ns/msg, 944,930.05 msgs/s
push_failed.  compare_exchange_weak failed: 11,093,155, buffer_is_full 725,795,864
pop_failed.  24,366,011

------------ producer Th count :10--------------
生产 : msg_count 10,000,000, total_ns 9580996979, 958.10 ns/msg, 1,043,732.72 msgs/s
消费 : msg_count 10,000,000, total_ns 9634918439, 963.49 ns/msg, 1,037,891.51 msgs/s
push_failed.  compare_exchange_weak failed: 15,042,465, buffer_is_full 719,829,610
pop_failed.  24,250,481
```