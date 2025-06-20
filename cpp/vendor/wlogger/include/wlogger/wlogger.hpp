#pragma once

#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <source_location>

#include "wlogger_impl.hpp"

namespace wtool {
namespace wlogger {

class Logger
{
    static inline std::once_flag initFlag;
    inline static LoggerWorkThread _impl;
    inline static Level _log_level {Level::INFO};

public:
    static Logger *instance()
    {
        static Logger ins {};
        return &ins;
    }
    static void initialize(const Config &cfg)
    {
        std::call_once(initFlag, [&cfg]() {
            _log_level = cfg.minLevel;
            _impl.configure(cfg);
            _impl.loop();

            instance()->log(std::source_location::current(), LOG_TAG, Level::INFO,
                            "Logger initialized");
        });
    }
    void setThreadName(std::string_view name) { Context ::setThreadName(std::string(name)); }

    template <typename... Args>
    void log(const std::source_location &loc, std::string tag, Level level,
             std::format_string<Args...> fmt, Args &&...args)
    {
        if (level < _log_level)
            return;

        try {
            LogMessage msg {std::format(fmt, std::forward<Args>(args)...), level,
                            Context(tag, loc)};

            constexpr int MAX_RETRIES = 100;
            int retries = 0;
            while (!LoggerData::buffer.enqueue(std::move(msg))) {
                if (++retries > MAX_RETRIES) {
                    std::this_thread::yield();
                    retries = 0;
                }
            }
            LoggerData::perf.producer();
        }
        catch (const std::exception &e) {
            std::cerr << "Logging error: " << e.what() << std::endl;
        }
    }

    template <typename... Args>
    void trace(std::string tag, const std::source_location &loc, std::format_string<Args...> fmt,
               Args &&...args)
    {
        log(loc, tag, Level::TRACE, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(std::string tag, const std::source_location &loc, std::format_string<Args...> fmt,
               Args &&...args)
    {
        log(loc, tag, Level::DEBUG, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(std::string tag, const std::source_location &loc, std::format_string<Args...> fmt,
              Args &&...args)
    {
        log(loc, tag, Level::INFO, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warning(std::string tag, const std::source_location &loc, std::format_string<Args...> fmt,
                 Args &&...args)
    {
        log(loc, tag, Level::WARNING, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(std::string tag, const std::source_location &loc, std::format_string<Args...> fmt,
               Args &&...args)
    {
        log(loc, tag, Level::ERROR, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void fatal(std::string tag, const std::source_location &loc, std::format_string<Args...> fmt,
               Args &&...args)
    {
        log(loc, tag, Level::FATAL, fmt, std::forward<Args>(args)...);
    }

    Logger() {};
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(Logger &&) = delete;
};

// helper macros for logging
#define LOG_TRACE(...)                                                                             \
    Logger::instance()->trace(LOG_TAG, std::source_location::current(), __VA_ARGS__)
#define LOG_DEBUG(...)                                                                             \
    Logger::instance()->debug(LOG_TAG, std::source_location::current(), __VA_ARGS__)
#define LOG_INFO(...)                                                                              \
    Logger::instance()->info(LOG_TAG, std::source_location::current(), __VA_ARGS__)
#define LOG_WARNING(...)                                                                           \
    Logger::instance()->warning(LOG_TAG, std::source_location::current(), __VA_ARGS__)
#define LOG_ERROR(...)                                                                             \
    Logger::instance()->error(LOG_TAG, std::source_location::current(), __VA_ARGS__)
#define LOG_FATAL(...)                                                                             \
    Logger::instance()->fatal(LOG_TAG, std::source_location::current(), __VA_ARGS__)

} // namespace wlogger
} // namespace wtool