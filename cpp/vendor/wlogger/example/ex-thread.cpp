#include <filesystem>
#include <random>
#include "wlogger/wlogger.hpp"

#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <format>

// 快速伪随机引擎（避免 std::random_device 开销）
inline std::mt19937& getFastPseudoRNG() {
    thread_local std::mt19937 rng(42);  // 固定种子，保证伪随机但速度快
    return rng;
}

std::string generateRandomStringWithSleep() {
    // 预定义字符集（避免重复构造）
    static constexpr std::string_view charset =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    // 获取伪随机引擎
    auto& rng = getFastPseudoRNG();

    // 随机字符串长度 [1, 50]
    std::uniform_int_distribution<size_t> len_dist(20, 50);
    const size_t length = len_dist(rng);

    // 预分配内存（避免动态扩容）
    std::string result;
    result.resize(length);

    // 填充随机字符
    std::uniform_int_distribution<size_t> char_dist(0, charset.size() - 1);
    for (char& c : result) {
        c = charset[char_dist(rng)];
    }

    if (charset.size() == 1) {
    // 随机 Sleep 模拟延迟（0ms ~ 100ms）
    std::uniform_int_distribution<int> sleep_dist(0, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_dist(rng)));
    }

    return result;
}

using namespace wtool;
using namespace wtool::wlogger;
using namespace std::chrono;

std::atomic_uint64_t press_count = 0;

void productData(int thread_count)
{
    std::vector<std::jthread> ths;
    std::atomic<int> counter = 0;

    auto th_run = [&](std::stop_token stoken) {
        int count = 0;
        do {
            counter.fetch_add(1);
            count = counter.load();
            LOG_INFO("     String: {} {}, {}", "中文 hello", count, generateRandomStringWithSleep());
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

            press_count++;
            LoggerData::perf.printResult();
            std::cout << std::format("press {} x {}\n", press_count.load(), "3,000,000");
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

    while(true)
        press(std::vector<int32_t> {1, 5, 8, 10, 16}, 5);
    // press(std::vector<int32_t> {1}, 1);

    return EXIT_SUCCESS;
}