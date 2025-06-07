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
    config.showThread = true;
    config.showFullPath = false;
    return config;
}

int main()
{
    auto config = getTestConfig();
    Logger::initialize(config);
#undef LOG_TAG
#define LOG_TAG "tag1"
    LOG_ERROR("main Starting basic tests...\n");
#undef LOG_TAG
#define LOG_TAG "tag2"
    Logger::instance()->setThreadName("main");
    // main test module
    LOG_ERROR("main Starting basic tests...\n");

    std::thread t1([] {
        Logger::instance()->setThreadName("t1");
        LOG_WARNING("     t1 String: {}", "hello");

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        LOG_FATAL("     t1 Multiple args: {}, {}, {}", 1, "two", 3.0);
    });
    t1.detach();

    std::thread t2([] {
        Logger::instance()->setThreadName("t2  000000099999996789");
        LOG_DEBUG("     t2 Right aligned: |{:>10}|", "right");

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        LOG_INFO("      t2 Hexadecimal: 0x{:X}", 255);
        Logger::instance()->setThreadName("ttt");
        LOG_INFO("t2 fff..\n");
    });
    t2.detach();

    std::thread t3([] { LOG_INFO("     t3 hhh hello"); });
    t3.detach();
    // run all tests
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    LOG_INFO("over.\n");

    return EXIT_SUCCESS;
}