
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

#### 2025.06.16

  push的时候，不使用原子操作，改为mutex。  

  <del>速度基本在 -> **[2,300,00.00,  3,300,000.00]msgs/s** 之间。</del>   
  <del>~~超过10线程，一次极端差的情况 **1,928,125.75 msgs/s**</del>

### 2025.06.18

  单线程可达到 2,800,000 msgs/s  
  多线程在 [1,300,000, 2,500,000] msgs/s

| Threads | Test 1 (msgs/s) | Test 2 (msgs/s) | Test 3 (msgs/s) | Test 4 (msgs/s) | Test 5 (msgs/s) |
|-----------------|----------------|----------------|----------------|----------------|----------------|
| 1              | 2,814,590.81   | 2,614,692.53   | 2,698,397.79   | 2,400,986.70   | 2,643,130.10   |
| 5              | 2,200,021.51   | 2,243,907.19   | 2,281,821.14   | 2,170,868.80   | 2,257,677.30   |
| 8              | 2,544,369.79   | 2,337,659.40   | 2,088,839.93   | 1,949,047.55   | 2,276,656.61   |
| 10             | 1,936,313.09   | 2,043,408.97   | 1,965,263.24   | 1,930,288.67   | 1,999,257.97   |
| 16             | 1,542,053.20   | 1,393,723.52   | 1,440,772.67   | 1,507,616.98   | 1,544,311.56   |

### 2025.06.21

使用moodycamel::ConcurrentQueue开源库后的新能，多线程性能无衰减；内存表现正常（无生产过快，内存不断增长的现象）

| Threads | Test 1 (msgs/s) | Test 2 (msgs/s) | Test 3 (msgs/s) | Test 4 (msgs/s) | Test 5 (msgs/s) |
|-----------------|----------------|----------------|----------------|----------------|----------------|
| 1              | 2,944,572.09   | 2,980,805.28   | 2,903,097.91   | 3,003,313.25   | 2,930,855.94   |
| 5              | 3,287,819.22   | 3,195,640.88   | 3,521,229.96   | 3,341,912.11   | 3,140,620.89   |
| 8              | 3,492,698.58   | 3,291,584.34   | 3,030,179.05   | 3,049,181.38   | 3,040,610.01   |
| 10             | 3,102,327.44   | 3,245,207.14   | 3,357,544.10   | 2,990,683.49   | 2,813,817.10   |
| 16             | 3,027,022.61   | 2,822,390.62   | 3,160,556.11   | 3,062,929.77   | 3,153,301.00   |