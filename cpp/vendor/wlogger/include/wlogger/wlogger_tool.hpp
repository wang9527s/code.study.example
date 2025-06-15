#pragma once

#include <chrono>
#include <format>
#include <string>

namespace wtool {
namespace wlogger {

class FastTimeFormatter
{
public:
    FastTimeFormatter()
        : last_time_sec_(0)
        , time_str_ {'\0'}
    {
    }

    // 格式化时间，返回内部缓冲区地址（仅单线程安全）
    const char *format(const std::chrono::system_clock::time_point &timestamp)
    {
        using namespace std::chrono;

        std::time_t t = system_clock::to_time_t(timestamp);
        auto us = duration_cast<microseconds>(timestamp.time_since_epoch()).count() % 1000000;

        if (t != last_time_sec_) {
            last_time_sec_ = t;
            std::tm tm_time;
#ifdef _WIN32
            localtime_s(&tm_time, &t);
#else
            localtime_r(&t, &tm_time);
#endif
            int year = tm_time.tm_year + 1900;
            int mon = tm_time.tm_mon + 1;
            int day = tm_time.tm_mday;
            int hour = tm_time.tm_hour;
            int min = tm_time.tm_min;
            int sec = tm_time.tm_sec;

            // 写入秒部分缓冲区，固定格式长度19字符 + 1终止符
            std::snprintf(time_str_, sizeof(time_str_), "%04d-%02d-%02d %02d:%02d:%02d", year, mon,
                          day, hour, min, sec);
        }

        // 组合最终时间字符串，格式：YYYY-MM-DD HH:MM:SS.uuuuuu
        // buf_大小 26字节足够: 19秒部分 + 1点 + 6微秒 + 1终止符
        std::snprintf(buf_, sizeof(buf_), "%s.%06d", time_str_, static_cast<int>(us));

        return buf_;
    }

private:
    std::time_t last_time_sec_;
    char time_str_[20]; // 秒部分缓存 "YYYY-MM-DD HH:MM:SS"
    char buf_[27];      // 最终格式化字符串缓存
};

inline static FastTimeFormatter g_time_format = {};

} // namespace wlogger
} // namespace wtool