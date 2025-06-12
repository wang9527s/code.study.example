#include <filesystem>
#include <random>
#include "include/wlogger.hpp"

using namespace wtool;
using namespace wtool::wlogger;
using namespace std::chrono;

void press(int th_count, int test_s = 10)
{
    LoggerData::total_msg_count = 0;
    std::cout << "\n--------------------------------------------\n";
    auto start = system_clock::now();
    std::vector<std::jthread> ths;
    auto th_run = [=](std::stop_token stoken) {
        int counter = 0;
        while (!stoken.stop_requested()) {
            LOG_WARNING("     String: {} {}", "中文 hello", ++counter);
        }
    };
    for (int i = 0; i < th_count; i++) {
        ths.push_back(std::jthread(th_run));
    }

    std::this_thread::sleep_for(seconds(test_s));
    for (int i = 0; i < ths.size(); i++) {
        ths[i].request_stop();
    }

    auto now = system_clock::now();
    std::this_thread::sleep_for(seconds(3));
    std::cout << "--------------------------------------------\n";
    std::cout << std::format("start: {:%F %T} ", zoned_time {current_zone(), start})
              << std::format("end  : {:%F %T}\n", zoned_time {current_zone(), now});

    int msg_count = LoggerData::total_msg_count;
    double avg_ns = (test_s * 1'000'000'000.0) / msg_count; // 总耗时除以条数 = 平均耗时 ns/msg
    double msgs_per_sec = msg_count / test_s;
    std::cout << std::format("count: {}, th count: {}, avg: {:.2f} ns/msg, {:.2f} msgs/s\n",
                             msg_count, th_count, avg_ns, msgs_per_sec);

    std::cout << "--------------------------------------------\n";
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

    press(1, 5);

    return EXIT_SUCCESS;
}