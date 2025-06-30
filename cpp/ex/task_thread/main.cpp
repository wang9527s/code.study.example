

#include <atomic>
#include <iostream>
#include "task_thread.hpp"

void PrintHello()
{
    std::cout << std::this_thread::get_id() << " 普通函数执行\n";
}

class MyWorker
{
public:
    void DoWork()
    {
        std::cout << std::this_thread::get_id() << " num = " << num << ", 成员函数执行\n";
    }
    int num = 10;
};

int main()
{
    std::cout << std::this_thread::get_id() << " main\n";
    TaskThread thread;

    thread.PostTask(PrintHello);

    std::string name = "john";
    thread.PostTask(
        [&name](std::string param) {
            name = "Tom";
            std::cout << std::this_thread::get_id() << " name is " << name << ", param is " << param
                      << " lambda 执行\n";
        },
        name);

    std::thread th([&] {
        std::cout << std::this_thread::get_id() << " th\n";
        // 延迟执行（2秒后）
        thread.PostDelayedTask(
            [] { std::cout << std::this_thread::get_id() << " 延迟 2 秒执行\n"; }, 2000);
        // 成员函数
        MyWorker worker;
        worker.num = 100;
        thread.PostTask(std::bind(&MyWorker::DoWork, &worker));
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    th.detach();

    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
}