#include <random>
#include "include/wlogger.hpp"

using namespace wtool;
using namespace wtool::wlogger;

// unified configuration
wlogger::Config getTestConfig()
{
    wlogger::Config config;
    config.maxFileSize = 5 * 1024 * 1024; // 5mb
    config.maxFiles = 3;
    config.logDir = "test_logs";
    config.filePrefix = "basic_test";
    config.minLevel = wlogger::Level::TRACE;
    config.consoleOutput = true;
    config.fileOutput = true;
    config.useColors = true;
    config.showTimestamp = true;
    config.showThreadId = true;
    config.showSourceLocation = true;
    config.showModuleName = true;
    config.showFullPath = false;
    return config;
}

int main()
{
    // initialize logger
    auto config = getTestConfig();
    Logger::initialize(config);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    Logger::getInstance()->setThreadName("BasicTest");
    // main test module
    LOG_INFO("main Starting basic tests...\n");

    std::thread t1([] {
        {
            Logger::getInstance()->setThreadName("Formatting");
            LOG_INFO("     t1 String: {}", "hello");
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            LOG_INFO("     t1 Multiple args: {}, {}, {}", 1, "two", 3.0);
        }
    });
    t1.detach();

    std::thread t2([] {
        Logger::getInstance()->setThreadName("gggg");
        // complex formatting
        LOG_INFO("     t2 Test special characters: \\n, \\t, \\r");
        LOG_INFO("     t2 Right aligned: |{:>10}|", "right");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        LOG_INFO("      t2 Hexadecimal: 0x{:X}", 255);

        Logger::getInstance()->setThreadName("ttt");
        LOG_INFO("t2 fff..\n");
    });
    t2.detach();
    // run all tests
    std::this_thread::sleep_for(std::chrono::milliseconds(3500));

    LOG_INFO("over.\n");

    return EXIT_SUCCESS;
}