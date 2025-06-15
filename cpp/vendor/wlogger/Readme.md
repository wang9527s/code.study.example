
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

使用更精确的统计方式（新能略微降低），速度 -> **[3,000,000.00,  3,100,000.00]msgs/s**  

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

#### 0616

  push的时候，不使用原子操作，改为mutex。  
  速度基本在 -> **[2,300,00.00,  3,300,000.00]msgs/s** 之间。  
  超过10线程，一次极端差的情况 **1,928,125.75 msgs/s**

```
------------ producer Th count :1--------------
消费 : msg_count 10,000,000, total_ns 2927313052, 292.73 ns/msg, 3,416,102.01 msgs/s
消费 : msg_count 10,000,000, total_ns 2775567123, 277.56 ns/msg, 3,602,867.29 msgs/s
消费 : msg_count 10,000,000, total_ns 3567170037, 356.72 ns/msg, 2,803,342.68 msgs/s
消费 : msg_count 10,000,000, total_ns 2702228068, 270.22 ns/msg, 3,700,649.89 msgs/s

------------ producer Th count :5--------------
消费 : msg_count 10,000,000, total_ns 2860402774, 286.04 ns/msg, 3,496,011.15 msgs/s
消费 : msg_count 10,000,000, total_ns 3034910174, 303.49 ns/msg, 3,294,990.44 msgs/s
消费 : msg_count 10,000,000, total_ns 3006109218, 300.61 ns/msg, 3,326,559.11 msgs/s
消费 : msg_count 10,000,000, total_ns 3116317620, 311.63 ns/msg, 3,208,915.53 msgs/s

------------ producer Th count :8--------------
消费 : msg_count 10,000,000, total_ns 3362389052, 336.24 ns/msg, 2,974,075.83 msgs/s
消费 : msg_count 10,000,000, total_ns 3565391888, 356.54 ns/msg, 2,804,740.77 msgs/s
消费 : msg_count 10,000,000, total_ns 3149953272, 315.00 ns/msg, 3,174,650.27 msgs/s
消费 : msg_count 10,000,000, total_ns 3523149675, 352.31 ns/msg, 2,838,369.33 msgs/s

------------ producer Th count :10--------------
消费 : msg_count 10,000,000, total_ns 3431514606, 343.15 ns/msg, 2,914,165.07 msgs/s
消费 : msg_count 10,000,000, total_ns 3535891815, 353.59 ns/msg, 2,828,140.83 msgs/s
消费 : msg_count 10,000,000, total_ns 4100973658, 410.10 ns/msg, 2,438,445.31 msgs/s
消费 : msg_count 10,000,000, total_ns 3843922929, 384.39 ns/msg, 2,601,508.98 msgs/s

------------ producer Th count :16--------------
消费 : msg_count 10,000,000, total_ns 4393391007, 439.34 ns/msg, 2,276,146.14 msgs/s
消费 : msg_count 10,000,000, total_ns 4319136362, 431.91 ns/msg, 2,315,277.68 msgs/s
消费 : msg_count 10,000,000, total_ns 4338013109, 433.80 ns/msg, 2,305,202.81 msgs/s
消费 : msg_count 10,000,000, total_ns 5186383707, 518.64 ns/msg, 1,928,125.75 msgs/s








------------ producer Th count :1--------------



------------ producer Th count :5--------------



------------ producer Th count :8--------------



------------ producer Th count :10--------------



------------ producer Th count :16--------------



------------ producer Th count :1--------------


------------ producer Th count :5--------------


------------ producer Th count :8--------------


------------ producer Th count :10--------------

------------ producer Th count :16--------------


------------ producer Th count :1--------------



------------ producer Th count :5--------------



------------ producer Th count :8--------------


------------ producer Th count :10--------------



------------ producer Th count :16--------------

```