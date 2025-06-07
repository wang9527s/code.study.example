#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <source_location>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace wtool {

std::string format_size_kb_mb(size_t bytes)
{
    constexpr size_t KB = 1024;
    constexpr size_t MB = 1024 * KB;

    double size = static_cast<double>(bytes);
    std::string unit = "B";

    if (bytes >= MB) {
        size = size / MB;
        unit = "MB";
    }
    else if (bytes >= KB) {
        size = size / KB;
        unit = "KB";
    }

    return std::to_string(bytes) + unit;
}

namespace wlogger {

enum class Level { TRACE, DEBUG, INFO, WARNING, ERROR, FATAL, STEP };

// logger configuration
struct Config {
    std::string logDir {"logs"};           // log directory
    std::string filePrefix {"app"};        // log file prefix
    size_t maxFileSize {10 * 1024 * 1024}; // max single file size (10MB)
    size_t maxFiles {5};                   // max number of files to keep
    Level minLevel {Level::INFO};          // minimum log level
    bool consoleOutput {true};             // enable console output
    bool fileOutput {true};                // enable file output
    bool useColors {true};                 // enable colored output
    bool showTimestamp {true};             // show timestamp in logs
    bool showThreadId {true};              // show thread id in logs
    bool showSourceLocation {true};        // show source location in logs
    bool showModuleName {true};            // show module name in logs
    bool showFullPath {false};             // show full file paths in logs
};

// log context information
struct Context {

    std::string module;       // module name
    std::string function;     // function name
    std::string file;         // file name
    int line;                 // line number
    std::thread::id threadId; // thread id

    Context(const std::source_location &loc = std::source_location::current())
        : module(currentThreadName)
        , function(loc.function_name())
        , file(loc.file_name())
        , line(loc.line())
        , threadId(std::this_thread::get_id())
    {
        initThreadInfo();
    }
    void initThreadInfo() {
        if (cur_thread_id == 0) {
            
            cur_thread_id = _next_thread_id.fetch_add(1, std::memory_order_seq_cst);

            currentThreadName = std::to_string(cur_thread_id);
            if (cur_thread_id == 1) {
                currentThreadName = std::to_string(cur_thread_id) + " Main";
            }

            module = currentThreadName;
            std::cout << cur_thread_id << ", " << currentThreadName 
                << ", " << std::hash<std::thread::id> {}(threadId)
                << std::endl;
            if (cur_thread_id == 2) {
                std::cout << "helllo world\n";
            }
        }
    }

    static void setThreadName(std::string name) { currentThreadName = std::to_string(cur_thread_id) + " " + name; }

    inline static std::atomic<int> _next_thread_id = 1;

    inline static thread_local int cur_thread_id = {0};
    inline static thread_local std::string currentThreadName = {"Null"};
};

// log message structure
struct alignas(64) LogMessage {
    std::string message;
    Level level;
    Context context;
    std::chrono::system_clock::time_point timestamp;

    LogMessage() = default;

    LogMessage(const LogMessage &) = delete;
    LogMessage &operator=(const LogMessage &) = delete;

    LogMessage(LogMessage &&) noexcept = default;
    LogMessage &operator=(LogMessage &&) noexcept = default;

    LogMessage(std::string msg, Level lvl, Context ctx)
        : message(std::move(msg))
        , level(lvl)
        , context(std::move(ctx))
        , timestamp(std::chrono::system_clock::now())
    {
    }

    ~LogMessage() = default;
};

// ring buffer implementation
static constexpr size_t BUFFER_SIZE = 1 << 17; // 2^17 = 131,072
struct alignas(64) RingBuffer {
    alignas(64) std::array<LogMessage, BUFFER_SIZE> messages;
    alignas(64) std::atomic<size_t> head {0}; // cache line 1
    alignas(64) std::atomic<size_t> tail {0}; // cache line 2

    bool push(LogMessage &&msg)
    {
        auto current_tail = tail.load(std::memory_order_relaxed);
        auto next_tail = (current_tail + 1) & (BUFFER_SIZE - 1);

        if (next_tail == head.load(std::memory_order_relaxed)) {
            if (next_tail == head.load(std::memory_order_acquire)) {
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

        if (current_head == tail.load(std::memory_order_relaxed)) {
            if (current_head == tail.load(std::memory_order_acquire)) {
                return false;
            }
        }

        msg = std::move(messages[current_head]);
        head.store((current_head + 1) & (BUFFER_SIZE - 1), std::memory_order_release);
        return true;
    }
};
} // namespace wlogger
} // namespace wtool