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

namespace wlogger {

class RingBuffer;
class PerlData;
struct LoggerData {
    static RingBuffer buffer;
    static PerlData perf;
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
        std::string_view file(context._loc.file_name());
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
    alignas(64) std::array<LogMessage, ConstData::Range_Buffer_Size> messages;
    alignas(64) std::atomic<size_t> head {0};
    alignas(64) std::atomic<size_t> tail {0};
    std::mutex _mutex;

    bool push(LogMessage &&msg)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        size_t current_tail = tail.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) & (ConstData::Range_Buffer_Size - 1);

        if (next_tail == head.load(std::memory_order_acquire)) {
            LoggerData::perf.err_count[PerlData::push_buff_is_full]++;
            return false;
        }
        tail.store(next_tail, std::memory_order_relaxed);
        // 如果此句在mutex的保护外，数据会被篡改
        //     [1970-01-01 08:00:00.000000] [TRACE] [Default] [wlogger_range_buffer.hpp:149]
        new (&messages[current_tail]) LogMessage(std::move(msg));

        return true;
    }

    bool pop(LogMessage &msg)
    {
        auto current_head = head.load(std::memory_order_relaxed);
        if (current_head == tail.load(std::memory_order_acquire)) {
            LoggerData::perf.err_count[PerlData::pop_buff_is_empty]++;
            return false;
        }

        msg = std::move(messages[current_head]);
        head.store((current_head + 1) & (ConstData::Range_Buffer_Size - 1),
                   std::memory_order_release);
        return true;
    }
};

RingBuffer LoggerData::buffer {};
PerlData LoggerData::perf = {};

} // namespace wlogger
} // namespace wtool