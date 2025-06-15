#include <filesystem>
#include <random>
#include "wlogger/wlogger.hpp"

using namespace wtool;
using namespace wtool::wlogger;
using namespace std::chrono;

void press(int th_count)
{
    std::this_thread::sleep_for(seconds(2));
    std::cout << std::format("\n------------ producer Th count :{}--------------\n", th_count);

    LoggerData::perf.clear();
    auto start = system_clock::now();
    std::vector<std::jthread> ths;
    std::atomic<int> counter = 0;
    auto th_run = [&](std::stop_token stoken) {
        int count = 0;
        do {
            counter.fetch_add(1);
            count = counter.load();
            LOG_INFO("     String: {} {}", "中文 hello", count);
        } while (!stoken.stop_requested() && count < PerlData::end_msg_idx);
    };
    for (int i = 0; i < th_count; i++) {
        ths.push_back(std::jthread(th_run));
    }

    for (int i = 0; i < ths.size(); i++) {
        ths[i].join();
    }
    std::this_thread::sleep_for(seconds(5));

    auto now = system_clock::now();

    LoggerData::perf.printResult();
}

int main()
{
    std::filesystem::remove("./logs/app.log");
    wlogger::Config config;
    config.minLevel = wlogger::Level::TRACE;
    config.fileOutput = true;
    config.consoleOutput = false;
    config.useColors = true;
    config.showThread = false;
    config.showFullPath = false;
    Logger::initialize(config);

    press(1);
    press(5);
    press(8);
    press(10);
    press(16);

    return EXIT_SUCCESS;
}