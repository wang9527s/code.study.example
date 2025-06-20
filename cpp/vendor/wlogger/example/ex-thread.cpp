#include <filesystem>
#include <random>
#include "wlogger/wlogger.hpp"

using namespace wtool;
using namespace wtool::wlogger;
using namespace std::chrono;

void productData(int thread_count)
{
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
    for (int i = 0; i < thread_count; i++) {
        ths.push_back(std::jthread(th_run));
    }

    for (int i = 0; i < ths.size(); i++) {
        ths[i].join();
    }
}

void press(std::vector<int32_t> th_counts, int32_t repeat_count = 1)
{
    for (int thread_count : th_counts) {
        std::cout << std::format("\n------------ producer Th count :{}--------------\n",
                                 thread_count);
        for (int i = 0; i < repeat_count; i++) {
            std::this_thread::sleep_for(milliseconds(100));
            LoggerData::perf.clear();

            // 生产数据
            productData(thread_count);

            do {
                std::this_thread::sleep_for(milliseconds(100));
            } while (!LoggerData::perf.consumer_over());

            LoggerData::perf.printResult();
        }
    }
}

int main()
{
    std::filesystem::remove("./logs/app.log");
    wlogger::Config config;
    config.minLevel = wlogger::Level::TRACE;
    config.fileOutput = false;
    config.consoleOutput = false;
    config.useColors = true;
    config.showThread = true;
    config.showFullPath = false;
    Logger::initialize(config);

    press(std::vector<int32_t> {1, 5, 8, 10, 16}, 5);
    // press(std::vector<int32_t> {1}, 1);

    return EXIT_SUCCESS;
}