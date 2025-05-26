#include <sigslot/signal.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <functional>
#include <atomic>

std::mutex log_mutex;
void log(std::string slog, bool no_thread_id = false) {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (no_thread_id) {
        std::cout << slog << std::endl;
    }
    else {
        std::cout << "[thread: " << std::this_thread::get_id() << "] " << slog << std::endl;
    }
}

int main() {
    log("main Thread");

    {
        // 槽函数运行在信号所在线程
        sigslot::signal<int> sig; 

        log("connect");
        sig.connect([](int value) {
            log("slot, receive value: " + std::to_string(value));
        });

        std::thread t1([&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            log("emit sig(456)");
            sig(456);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        });
        t1.detach();

        // 触发信号
        log("emit sig(123)");
        sig(123);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    log("", true);
    log("/* *********************************** */", true);

    {
        // 槽函数还是运行在sig坐在线程
        // 槽函数中修改原子变量，通知另外一个工作线程开始执行任务
        std::atomic_bool signal_received;
        signal_received = false;

        sigslot::signal<int> sig;

        // 设定一个工作线程，在这个线程中执行槽函数
        std::thread worker([&]() {
            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (signal_received) {
                    log("worker thread, work...");
                    signal_received = false;
                }
            }
        });
        worker.detach();

        log("connect");
        sig.connect([&](int value) {
            log("receive value: " + std::to_string(value));
            signal_received = true;
        });

        std::thread t1([&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            log("emit sig(456)");
            sig(456);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        });
        t1.detach();

        log("emit sig(123)");
        sig(123);

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 0;
}
