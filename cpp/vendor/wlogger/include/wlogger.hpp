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

#include "wlogger_impl.hpp"

namespace wtool {
namespace wlogger {

// struct LogMessage;
class Logger
{

    // terminal colors
    static constexpr std::array<const char *, 10> COLORS = {
        "\033[0m",   // reset
        "\033[30m",  // black
        "\033[31m",  // red
        "\033[32m",  // green
        "\033[33m",  // yellow
        "\033[34m",  // blue
        "\033[35m",  // magenta
        "\033[36m",  // cyan
        "\033[37m",  // white
        "\033[1;31m" // bright red
    };

    static constexpr size_t COLOR_RESET = 0;   // reset
    static constexpr size_t COLOR_TRACE = 7;   // cyan
    static constexpr size_t COLOR_DEBUG = 6;   // magenta
    static constexpr size_t COLOR_INFO = 3;    // green
    static constexpr size_t COLOR_WARNING = 4; // yellow
    static constexpr size_t COLOR_ERROR = 2;   // red
    static constexpr size_t COLOR_FATAL = 9;   // bright red
    static constexpr size_t COLOR_STEP = 5;    // blue

    static constexpr std::array<std::string_view, 7> LEVEL_STRINGS = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "STEP"};

    const char *getLevelColor(Level level) const
    {
        switch (level) {
        case Level::TRACE:
            return COLORS[COLOR_TRACE];
        case Level::DEBUG:
            return COLORS[COLOR_DEBUG];
        case Level::INFO:
            return COLORS[COLOR_INFO];
        case Level::WARNING:
            return COLORS[COLOR_WARNING];
        case Level::ERROR:
            return COLORS[COLOR_ERROR];
        case Level::FATAL:
            return COLORS[COLOR_FATAL];
        case Level::STEP:
            return COLORS[COLOR_STEP];
        default:
            return COLORS[COLOR_RESET];
        }
    }
    // member variables
    Config config;
    RingBuffer buffer;
    mutable std::shared_mutex configMutex; // for config changes
    std::ofstream logFile;
    std::jthread loggerThread;
    std::atomic<bool> running {true};
    std::atomic<size_t> currentFileSize {0};

    static inline std::once_flag initFlag;

    // private member functions
    Logger()
        : config(Config {})
        , running(true) {};
    void processLogs(std::stop_token st)
    {
        // batch size chosen to balance memory usage and throughput
        constexpr size_t BATCH_SIZE = 4096;

        // pre-allocate buffers to avoid frequent reallocations
        std::vector<LogMessage> batchBuffer;
        batchBuffer.reserve(BATCH_SIZE);

        // allocate 1MB for each output buffer
        std::vector<char> fileBuffer;
        fileBuffer.reserve(1024 * 1024);
        std::vector<char> consoleBuffer;
        consoleBuffer.reserve(1024 * 1024);

        // lambda to process a batch of messages
        auto processBuffer = [&](std::span<LogMessage> buffer) {
            if (buffer.empty())
                return;

            // clear buffers for reuse
            fileBuffer.clear();
            consoleBuffer.clear();

            // process each message in the batch
            for (const auto &msg : buffer) {
                // handle file output
                if (config.fileOutput) {
                    formatLogMessage(msg, fileBuffer);
                    fileBuffer.push_back('\n');
                }

                // handle console output with optional colors
                if (config.consoleOutput) {
                    if (config.useColors) {
                        // add color prefix
                        std::string_view levelColor = getLevelColor(msg.level);
                        consoleBuffer.insert(consoleBuffer.end(), levelColor.begin(), levelColor.end());

                        // format message
                        formatLogMessage(msg, consoleBuffer);

                        // add color reset and newline
                        consoleBuffer.insert(consoleBuffer.end(), COLORS[0], COLORS[0] + strlen(COLORS[0]));
                        consoleBuffer.push_back('\n');
                    }
                    else {
                        formatLogMessage(msg, consoleBuffer);
                        consoleBuffer.push_back('\n');
                    }
                }
            }

            // write file buffer if needed
            if (config.fileOutput && logFile.is_open() && !fileBuffer.empty()) {
                logFile.write(fileBuffer.data(), fileBuffer.size());
                logFile.flush();
                currentFileSize += fileBuffer.size();
                // rotateLogFileIfNeeded();
            }

            // write console buffer if needed
            if (config.consoleOutput && !consoleBuffer.empty()) {
                std::cout.write(consoleBuffer.data(), consoleBuffer.size());
                std::cout.flush();
            }
        };

        // main processing loop
        while (!st.stop_requested()) {
            LogMessage msg;
            // collect messages until batch is full or queue is empty
            while (batchBuffer.size() < BATCH_SIZE && buffer.pop(msg)) {
                batchBuffer.push_back(std::move(msg));
            }

            if (!batchBuffer.empty()) {
                processBuffer(std::span<LogMessage>(batchBuffer.data(), batchBuffer.size()));
                batchBuffer.clear();
            }
            else {
                // sleep briefly when no messages are available
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        // process remaining messages before shutdown
        LogMessage msg;
        while (buffer.pop(msg)) {
            batchBuffer.push_back(std::move(msg));
            if (batchBuffer.size() >= BATCH_SIZE) {
                processBuffer(std::span<LogMessage>(batchBuffer.data(), batchBuffer.size()));
                batchBuffer.clear();
            }
        }

        // process final batch if any messages remain
        if (!batchBuffer.empty()) {
            processBuffer(std::span<LogMessage>(batchBuffer.data(), batchBuffer.size()));
        }
    }
    void formatLogMessage(const LogMessage &msg, std::vector<char> &buffer) noexcept
    {
        // estimate required space to minimize reallocations
        // base size (256) accounts for timestamps, level, thread id etc
        const size_t estimated_size = 256 + msg.message.size();
        buffer.reserve(buffer.size() + estimated_size);

        // format timestamp if enabled
        if (config.showTimestamp)
            [[likely]]
            {
                auto time = std::chrono::system_clock::to_time_t(msg.timestamp);
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(msg.timestamp.time_since_epoch()) % 1000;

                char time_buffer[32];
                size_t time_len = std::strftime(time_buffer, sizeof(time_buffer), "[%Y-%m-%d %H:%M:%S.", std::localtime(&time));
                buffer.insert(buffer.end(), time_buffer, time_buffer + time_len);

                char ms_buffer[8];
                int ms_len = std::snprintf(ms_buffer, sizeof(ms_buffer), "%03d] ", static_cast<int>(ms.count()));
                buffer.insert(buffer.end(), ms_buffer, ms_buffer + ms_len);
            }

        // format log level
        const auto &level_str = LEVEL_STRINGS[static_cast<size_t>(msg.level)];
        buffer.push_back('[');
        buffer.insert(buffer.end(), level_str.begin(), level_str.end());
        buffer.insert(buffer.end(), {']', ' '});

        // format thread id if enabled
        if (config.showThreadId)
            [[likely]]
            {
                char thread_buffer[32];
                int thread_len = std::snprintf(thread_buffer, sizeof(thread_buffer), "[T-%zu] ",
                                               std::hash<std::thread::id> {}(msg.context.threadId));
                buffer.insert(buffer.end(), thread_buffer, thread_buffer + thread_len);
            }

        // format module name if enabled and not empty
        if (config.showModuleName && !msg.context.module.empty())
            [[likely]]
            {
                buffer.push_back('[');
                buffer.insert(buffer.end(), msg.context.module.begin(), msg.context.module.end());
                buffer.insert(buffer.end(), {']', ' '});
            }

        // format source location if enabled
        if (config.showSourceLocation)
            [[likely]]
            {
                std::string_view file(msg.context.file);
                if (!config.showFullPath)
                    [[likely]]
                    {
                        if (auto pos = file.find_last_of("/\\"); pos != std::string_view::npos)
                            [[likely]] { file = file.substr(pos + 1); }
                    }

                buffer.push_back('[');
                buffer.insert(buffer.end(), file.begin(), file.end());

                char line_buffer[16];
                int line_len = std::snprintf(line_buffer, sizeof(line_buffer), ":%d] ", msg.context.line);
                buffer.insert(buffer.end(), line_buffer, line_buffer + line_len);
            }

        // append the actual message content
        buffer.insert(buffer.end(), msg.message.begin(), msg.message.end());
    }

public:
    static Logger *getInstance()
    {
        static Logger instance {};
        return &instance;
    }
    static void initialize(const Config &cfg)
    {
        std::call_once(initFlag, [&cfg]() {
            getInstance()->configure(cfg); // configure logger

            getInstance()->loggerThread = std::jthread([](std::stop_token st) { getInstance()->processLogs(st); });

            getInstance()->log(std::source_location::current(), Level::INFO, "Logger initialized");
        });
    }

    void configure(const Config &cfg)
    {
        std::unique_lock lock(configMutex);

        // close the current log file if open
        if (logFile.is_open()) {
            logFile.close();
        }

        // update the configuration
        config = cfg;

        // reopen the log file with the new configuration
        if (config.fileOutput) {
            if (!std::filesystem::exists(config.logDir)) {
                std::filesystem::create_directories(config.logDir);
            }

            std::string filename = std::format("{}/{}.log", config.logDir, config.filePrefix);
            logFile.open(filename, std::ios::app);
            if (!logFile) {
                throw std::runtime_error(std::format("Failed to open log file: {}", filename));
            }

            currentFileSize = std::filesystem::file_size(filename);
            std::cout << "currentFileSize " << format_size_kb_mb(currentFileSize) << std::endl;
        }
    }
    void setLogLevel(Level level)
    {
        std::unique_lock lock(configMutex);
        config.minLevel = level;
    }
    void setThreadName(std::string_view name) { Context ::setThreadName(std::string(name)); }
    friend std::unique_ptr<Logger> std::make_unique<Logger>();

    // log method
    template <typename... Args>
    void log(const std::source_location &loc, Level level, std::format_string<Args...> fmt, Args &&... args)
    {
        if (level < config.minLevel)
            return;

        try {
            LogMessage msg {std::format(fmt, std::forward<Args>(args)...), level, Context(loc)};

            constexpr int MAX_RETRIES = 100;
            int retries = 0;
            while (!buffer.push(std::move(msg))) {
                if (++retries > MAX_RETRIES) {
                    std::this_thread::yield();
                    retries = 0;
                }
            }
        }
        catch (const std::exception &e) {
            std::cerr << "Logging error: " << e.what() << std::endl;
        }
    }

    template <typename... Args>
    void trace(const std::source_location &loc, std::format_string<Args...> fmt, Args &&... args)
    {
        log(loc, Level::TRACE, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(const std::source_location &loc, std::format_string<Args...> fmt, Args &&... args)
    {
        log(loc, Level::DEBUG, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(const std::source_location &loc, std::format_string<Args...> fmt, Args &&... args)
    {
        log(loc, Level::INFO, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warning(const std::source_location &loc, std::format_string<Args...> fmt, Args &&... args)
    {
        log(loc, Level::WARNING, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(const std::source_location &loc, std::format_string<Args...> fmt, Args &&... args)
    {
        log(loc, Level::ERROR, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void fatal(const std::source_location &loc, std::format_string<Args...> fmt, Args &&... args)
    {
        log(loc, Level::FATAL, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void step(int stepNum, const std::source_location &loc, std::format_string<Args...> fmt, Args &&... args)
    {
        log(loc, Level::STEP, "[Step {}] {}", stepNum, std::format(fmt, std::forward<Args>(args)...));
    }

    ~Logger()
    {
        try {
            running = false;
            if (loggerThread.joinable()) {
                loggerThread.request_stop();
                loggerThread.join();
            }
            if (logFile.is_open()) {
                logFile.close();
            }
        }
        catch (...) {
            // Ignore exceptions in destructor
        }
    }

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(Logger &&) = delete;
};

// helper macros for logging
#define LOG_TRACE(...) Logger::getInstance()->trace(std::source_location::current(), __VA_ARGS__)
#define LOG_DEBUG(...) Logger::getInstance()->debug(std::source_location::current(), __VA_ARGS__)
#define LOG_INFO(...) Logger::getInstance()->info(std::source_location::current(), __VA_ARGS__)
#define LOG_WARNING(...) Logger::getInstance()->warning(std::source_location::current(), __VA_ARGS__)
#define LOG_ERROR(...) Logger::getInstance()->error(std::source_location::current(), __VA_ARGS__)
#define LOG_FATAL(...) Logger::getInstance()->fatal(std::source_location::current(), __VA_ARGS__)
#define LOG_STEP(num, ...) Logger::getInstance()->step(num, std::source_location::current(), __VA_ARGS__)

} // namespace wlogger
} // namespace wtool