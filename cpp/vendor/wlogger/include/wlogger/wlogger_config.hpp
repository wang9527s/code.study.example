#pragma once

#include <array>
#include <string>
#include <string_view>

namespace wtool {
namespace wlogger {

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

} // namespace wlogger
} // namespace wtool