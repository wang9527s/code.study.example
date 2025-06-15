#pragma once

#include <array>
#include <atomic>
#include <format>
#include <thread>
#include <vector>

#include "perf.hpp"
#include "wlogger_config.hpp"
#include "wlogger_tool.hpp"

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
class PerlData;
struct LoggerData {
    static RingBuffer buffer;
    static PerlData perf;
    inline static constexpr bool enablePerfStat = false;

    inline static constexpr int Msg_Buffer_Size = 4096;
    inline static constexpr int Range_Buffer_Size = (1 << 17);
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
};

struct alignas(64) RingBuffer {
    alignas(64) std::array<LogMessage, LoggerData::Range_Buffer_Size> messages;
    alignas(64) std::atomic<size_t> head {0};
    alignas(64) std::atomic<size_t> tail {0};

    bool push(LogMessage &&msg)
    {
        size_t current_tail = tail.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) & (LoggerData::Range_Buffer_Size - 1);

        if (next_tail == head.load(std::memory_order_acquire)) {
            LoggerData::perf.push_failed_count_buffer_is_full++;
            return false;
        }

        if (!tail.compare_exchange_weak(current_tail, next_tail, std::memory_order_acquire,
                                        std::memory_order_relaxed)) {
            LoggerData::perf.push_failed_count++;
            return false;
        }

        new (&messages[current_tail]) LogMessage(std::move(msg));
        return true;
    }

    bool pop(LogMessage &msg)
    {
        auto current_head = head.load(std::memory_order_relaxed);
        if (current_head == tail.load(std::memory_order_acquire)) {
            LoggerData::perf.pop_failed_count++;
            return false;
        }

        msg = std::move(messages[current_head]);
        head.store((current_head + 1) & (LoggerData::Range_Buffer_Size - 1),
                   std::memory_order_release);
        return true;
    }
};

RingBuffer LoggerData::buffer = RingBuffer();
PerlData LoggerData::perf = {};

} // namespace wlogger
} // namespace wtool