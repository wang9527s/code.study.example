#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class TaskThread
{
public:
    TaskThread()
        : running_(true)
        , thread_(&TaskThread::ThreadLoop, this)
    {
    }

    ~TaskThread()
    {
        running_ = false;
        cv_.notify_all();
        if (thread_.joinable())
            thread_.join();
    }

    // 支持变参的任务提交（立即执行）
    template <typename F, typename... Args>
    void PostTask(F &&f, Args &&... args)
    {
        PostDelayedTask(std::forward<F>(f), 0, std::forward<Args>(args)...);
    }

    template <typename F, typename... Args>
    void PostDelayedTask(F &&f, int delay_ms, Args &&... args)
    {
        auto exec_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay_ms);
        auto bound_task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            task_queue_.emplace(TaskItem {std::move(bound_task), exec_time});
        }
        cv_.notify_one();
    }

private:
    struct TaskItem {
        std::function<void()> func;
        std::chrono::steady_clock::time_point exec_time;
        std::size_t id;
        bool operator>(const TaskItem &other) const { return exec_time > other.exec_time; }
    };

    void ThreadLoop()
    {
        while (running_) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(mutex_);

                if (task_queue_.empty()) {
                    cv_.wait(lock);
                    continue;
                }

                auto now = std::chrono::steady_clock::now();
                auto &next_task = task_queue_.top();

                if (next_task.exec_time > now) {
                    cv_.wait_until(lock, next_task.exec_time);
                    continue;
                }

                task = std::move(next_task.func);
                task_queue_.pop();
            }

            if (task)
                task();
        }
    }

    std::atomic<bool> running_;
    std::thread thread_;
    std::condition_variable cv_;
    std::mutex mutex_;
    // 支持排序的队列
    std::priority_queue<TaskItem, std::vector<TaskItem>, std::greater<>> task_queue_;
};