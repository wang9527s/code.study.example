#pragma once

#include <array>
#include <atomic>
#include <thread>
#include <vector>

#include "wlogger_helper.hpp"

#ifndef LOG_TAG
#define LOG_TAG "Default"
#endif

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

class TimeUse
{
    std::string _tag;

public:
    TimeUse(std::string tag)
        : total_duration_ns(0)
        , _tag(tag)
        , count(0)
        , current_duration_ns(0)
    {
    }

    void start() { start_time = std::chrono::system_clock::now(); }

    void end()
    {
        auto end_time = std::chrono::system_clock::now();
        current_duration_ns
            = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
        total_duration_ns += current_duration_ns;
        ++count;
    }

    void print() const
    {
        double avg_ns = static_cast<double>(total_duration_ns) / count;
        double msgs_per_sec = 1'000'000'000.0 / avg_ns;

        std::cout << std::format("{} :  avg: {:.2f} ns/msg, {:.2f} msgs/s\n", _tag, avg_ns,
                                 msgs_per_sec);
    }

private:
    std::chrono::time_point<std::chrono::system_clock> start_time;
    long long current_duration_ns;
    long long total_duration_ns;
    int count;
};

namespace wlogger {

class RingBuffer;
struct LoggerData {
    static RingBuffer buffer;
    inline static constexpr size_t Msg_Buffer_Size = 4096;
    inline static constexpr size_t Range_Buffer_Size = 1 << 17; // 2^17 = 131,072

    inline static constexpr bool enablePerfStat = false;

    inline static size_t total_msg_count = 0;
};

enum class Level { TRACE, DEBUG, INFO, WARNING, ERROR, FATAL };

const char *Color_Normal = "\033[0m";
const char *Color_Warning = "\033[1;34m"; // 蓝
const char *Color_Error = "\033[1;33m";   // 黄
const char *Color_Fatal = "\033[1;31m";   // 红

static constexpr std::array<std::string_view, 6> LEVEL_STRINGS
    = {"TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL"};

const char *getLevelColor(Level level)
{
    switch (level) {
    case Level::WARNING:
        return Color_Warning;
    case Level::ERROR:
        return Color_Error;
    case Level::FATAL:
        return Color_Fatal;

    default:
        return Color_Normal;
    }
}

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
    bool showThread {true};                // show thread id in logs
    bool showFullPath {false};             // show full file paths in logs
};

// log context information
struct Context {
    std::string tag;
    std::string threadName;
    std::source_location _loc;

    Context(std::string stag = LOG_TAG,
            const std::source_location &loc = std::source_location::current())
        : tag(stag)
        , _loc(loc)
        , threadName(currentThreadName)
    {
        init();
    }

    void init()
    {
        if (cur_thread_id == 0) {
            cur_thread_id = _next_thread_id.fetch_add(1, std::memory_order_seq_cst);
            initThreadName("");
            threadName = currentThreadName;
        }
    }
    static void initThreadName(std::string name)
    {
        name = "(" + std::to_string(cur_thread_id) + ") " + name;

        const int maxLength = 16;
        char buf[maxLength];
        size_t len = std::min(name.size(), size_t(maxLength));
        std::memcpy(buf, name.data(), len);
        if (len < maxLength) {
            std::memset(buf + len, ' ', maxLength - len);
        }
        currentThreadName = std::string(buf, maxLength);
    }
    static void setThreadName(std::string name)
    {
        if (cur_thread_id == 0) {
            cur_thread_id = _next_thread_id.fetch_add(1, std::memory_order_seq_cst);
        }
        initThreadName(name);
    }

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

    std::string format_str(bool showFullPath, bool showThread) const
    {
        const auto &level_str = LEVEL_STRINGS[static_cast<size_t>(level)];
        std::string file(context._loc.file_name());
        if (!showFullPath) [[likely]] {
            if (auto pos = file.find_last_of("/\\"); pos != std::string_view::npos) [[likely]] {
                file = file.substr(pos + 1);
            }
        }
        if (showThread) {
            return std::format("[{}] [{}] [{}] [{}] [{}:{}] {}", g_time_format.format(timestamp),
                               level_str, context.threadName, context.tag, file,
                               context._loc.line(), message);
        }
        else {
            return std::format("[{}] [{}] [{}] [{}:{}] {}", g_time_format.format(timestamp),
                               level_str, context.tag, file, context._loc.line(), message);
        }

        return "";
    }

private:
    /*
        // 取消此函数，放到format_str中，大概有50ns的提升（因该是少一次format的新能提升）
        std::string formatTime() const
        {
            // 单次获取毫秒计数（最快方式）
            const auto ms_since_epoch
                =
       std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()); const
       auto sec_part = std::chrono::floor<std::chrono::seconds>(timestamp);

            // 此行大概有 200ns的耗时
            return std::format("{:%Y-%m-%d %H:%M:%S}.{:03d}", sec_part, ms_since_epoch.count() %
       1000);
        }
    */
};

struct alignas(64) RingBuffer {
    alignas(64) std::array<LogMessage, LoggerData::Range_Buffer_Size> messages;
    alignas(64) std::atomic<size_t> head {0}; // cache line 1
    alignas(64) std::atomic<size_t> tail {0}; // cache line 2

    bool push(LogMessage &&msg)
    {
        auto current_tail = tail.load(std::memory_order_relaxed);
        auto next_tail = (current_tail + 1) & (LoggerData::Range_Buffer_Size - 1);

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
        head.store((current_head + 1) & (LoggerData::Range_Buffer_Size - 1),
                   std::memory_order_release);
        return true;
    }
};

RingBuffer LoggerData::buffer = RingBuffer();

} // namespace wlogger
} // namespace wtool