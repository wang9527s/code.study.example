### 环形无锁队列

```cpp
struct alignas(64) RingBuffer
{
    alignas(64) std::array<LogMessage, BUFFER_SIZE> messages;
    alignas(64) std::atomic<size_t> head{0}; // cache line 1
    alignas(64) std::atomic<size_t> tail{0}; // cache line 2

    bool push(LogMessage &&msg)
    {
        auto current_tail = tail.load(std::memory_order_relaxed);

        // BUFFER_SIZE 是 2 的幂 情况下，等价于
        // next_tail = current_tail + 1 == UFFER_SIZE  ? 0 : current_tail + 1
        auto next_tail = (current_tail + 1) & (BUFFER_SIZE - 1);

        // 双重原子校验，兼顾性能和可靠性
        if (next_tail == head.load(std::memory_order_relaxed))
        {
            if (next_tail == head.load(std::memory_order_acquire))
            {
                return false;
            }
        }

        new (&messages[current_tail]) LogMessage(std::move(msg));
        tail.store(next_tail, std::memory_order_release);
        return true;
    }

    bool pop(LogMessage &msg)
    {
        auto current_head = head.load(std::memory_order_relaxed);

        if (current_head == tail.load(std::memory_order_relaxed))
        {
            if (current_head == tail.load(std::memory_order_acquire))
            {
                return false;
            }
        }

        msg = std::move(messages[current_head]);
        head.store((current_head + 1) & (BUFFER_SIZE - 1),
                   std::memory_order_release);
        return true;
    }
};
```

### std::jthread

```cpp
//  g++-13 -std=c++20  -pthread main.cpp

#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Starting jthread...\n";

    std::jthread worker([=](std::stop_token stoken) {
        int counter = 0;
        while (!stoken.stop_requested()) {
            std::cout << "Working... " << counter++ << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        std::cout << "Stop requested, exiting thread.\n";
    });  // 自动传递 stop_token

    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "Requesting stop...\n";
    worker.request_stop();  // 通知线程停止

    // 无需调用 join()，jthread 变量 析构时会自动等待线程结束
    std::cout << "Main thread done.\n";
    return 0;
}
```

### 日志等级

**如何做到release程序不输出debug日志？ **

+ 通过宏

主流日志库（如 spdlog、glog、Boost.Log）均内置日志级别控制，在代码中设置日志级别，然后日志库中判断，不符合要求的日志不输出，从而控制release不输出debug日志

```cpp
#ifdef NDEBUG
    spdlog::set_level(spdlog::level::info);  // Release 只输出 INFO 及以上
#else
    spdlog::set_level(spdlog::level::debug); // Debug 输出所有日志
#endif
```

+ 环境变量/配置文件

程序启动的时候，读取环境变量/日志文件，从而控制日志的输出级别。  
这样可以在不变更程序的情况下动态调整日志级别。