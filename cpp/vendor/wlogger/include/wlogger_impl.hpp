#pragma once

#include "wlogger_header.hpp"

namespace wtool {
namespace wlogger {

class LoggerWorkThread
{
public:
    LoggerWorkThread()
    {
        _MsgBuffer.reserve(LoggerData::Msg_Buffer_Size);
        _fileBuffer.reserve(1024 * 1024);
        _consoleBuffer.reserve(1024 * 1024);
    }
    ~LoggerWorkThread()
    {
        try {
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
    void loop()
    {
        loggerThread = std::jthread([&](std::stop_token st) { worker(st); });
    }
    void configure(const Config &cfg)
    {
        if (logFile.is_open()) {
            logFile.close();
        }

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
        }
    }

private:
    void porcessMsgBuffer()
    {
        if (_MsgBuffer.empty())
            return;

        for (const auto &msg : _MsgBuffer) {
            static TimeUse t("msg.format");
            static TimeUse t2("onemsg ");
            static int count = 0;
            ++count;
            if (count % 100 == 0) {
                t.start();
                t2.start();
            }
            std::string msg_format = msg.format_str(config.showFullPath, config.showThread);
            if (count % 100 == 0) {
                t.end();
            }

            LoggerData::total_msg_count++;
            if (config.fileOutput) {
                // 在_fileBuffer后进行append
                std::format_to(std::back_inserter(_fileBuffer), "{}\n", msg_format);
            }

            if (config.consoleOutput) {
                if (config.useColors) {
                    std::format_to(std::back_inserter(_consoleBuffer), "{}{}{}\n",
                                   getLevelColor(msg.level), msg_format, Color_Normal);
                }
                else {
                    std::format_to(std::back_inserter(_consoleBuffer), "{}\n", msg_format);
                }
            }
            if (count % 100 == 0) {
                t2.end();
            }
            if (count % 8000 == 0) {
                // print 需要在统计之后
                t.print();
                t2.print();
            }
        }

        // write file buffer if needed
        if (config.fileOutput && logFile.is_open() && !_fileBuffer.empty()) {
            logFile.write(_fileBuffer.data(), _fileBuffer.size());
            logFile.flush();
            currentFileSize += _fileBuffer.size();
            // rotateLogFileIfNeeded();
        }

        // write console buffer if needed
        if (config.consoleOutput && !_consoleBuffer.empty()) {
            std::cout.write(_consoleBuffer.data(), _consoleBuffer.size());
            std::cout.flush();
        }

        _MsgBuffer.clear();
        _fileBuffer.clear();
        _consoleBuffer.clear();
    }
    void worker(std::stop_token st)
    {
        while (!st.stop_requested()) {
            LogMessage msg;
            // collect messages until batch is full or queue is empty
            while (_MsgBuffer.size() < LoggerData::Msg_Buffer_Size && LoggerData::buffer.pop(msg)) {
                _MsgBuffer.push_back(std::move(msg));
            }

            if (!_MsgBuffer.empty()) {
                porcessMsgBuffer();
            }
            else {
                std::this_thread::yield();
            }
        }

        // process remaining messages before shutdown
        LogMessage msg;
        while (LoggerData::buffer.pop(msg)) {
            _MsgBuffer.push_back(std::move(msg));
        }
        porcessMsgBuffer();
    }

private:
    Config config {};
    std::ofstream logFile;
    std::jthread loggerThread;
    std::atomic<size_t> currentFileSize {0};

    std::vector<LogMessage> _MsgBuffer;
    std::vector<char> _fileBuffer;
    std::vector<char> _consoleBuffer;
};

} // namespace wlogger
} // namespace wtool