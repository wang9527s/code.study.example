
#include <atomic>
#include <chrono>
#include <string>

#define Million 100'0000

namespace wtool {
namespace wlogger {

struct CommaNumpunct : std::numpunct<char> {
protected:
    char do_thousands_sep() const override { return ','; }
    std::string do_grouping() const override { return "\3"; }
};

std::string formatNum(auto value, int precision = 2)
{
    std::stringstream ss;
    ss.imbue(std::locale(std::locale::classic(), new CommaNumpunct()));

    // 判断是否是浮点数
    if constexpr (std::is_floating_point_v<decltype(value)>) {
        ss << std::fixed << std::setprecision(precision);
    }

    ss << value;
    return ss.str();
};

class PerlData
{
    long long producer_s = 0;
    long long producer_e = 0;
    uint64_t producer_ns = 0;

    long long consumer_s = 0;
    long long consumer_e = 0;
    uint64_t consumer_ns = 0;

public:
    static constexpr uint64_t start_msg_idx = 5;
    static constexpr uint64_t end_msg_idx = 10 * Million + start_msg_idx;

    enum Failed {
        push_buff_is_full,
        pop_buff_is_empty
    };

    std::atomic<uint64_t> producer_msg_count = 0;

    uint64_t consumer_msg_count = 0;
    uint32_t err_count [8] = {0};

public:
    void clear()
    {
        // std::cout << std::format("old: producer={} consumer={} push_fail={} pop_fail={}\n",
        //                          formatNum(producer_msg_count.load()),
        //                          formatNum(consumer_msg_count), push_failed_count.load(),
        //                          pop_failed_count);

        producer_msg_count = 0;
        consumer_msg_count = 0;

        std::fill(std::begin(err_count), std::end(err_count), 0);


        consumer_ns = producer_ns = 0;
        // std::cout << std::format("new: producer={} consumer={} push_fail={} pop_fail={}\n",
        //                          formatNum(producer_msg_count.load()),
        //                          formatNum(consumer_msg_count), push_failed_count.load(),
        //                          pop_failed_count);
    }

    void producer()
    {
        using namespace std::chrono;
        const int32_t count = ++producer_msg_count;
        if (count == start_msg_idx) {
            // std::cout << "start producer\n";
            auto s = system_clock::now();
            producer_s = duration_cast<nanoseconds>(s.time_since_epoch()).count();
        }
        else if (count == end_msg_idx) {
            auto e = system_clock::now();
            // std::cout << "stop producer\n";
            producer_e = duration_cast<nanoseconds>(e.time_since_epoch()).count();
            producer_ns = producer_e - producer_s;
        }
    }
    void consumer()
    {
        const uint32_t count = ++consumer_msg_count;
        using namespace std::chrono;

        if (count == start_msg_idx) {
            // std::cout << "start consumer\n";
            auto s = system_clock::now();
            consumer_s = duration_cast<nanoseconds>(s.time_since_epoch()).count();
        }
        else if (count == end_msg_idx) {
            // std::cout << "stop consumer\n";
            auto e = system_clock::now();
            consumer_e = duration_cast<nanoseconds>(e.time_since_epoch()).count();
            consumer_ns = consumer_e - consumer_s;
        }
    }

    void printResult()
    {
        int msg_count = end_msg_idx - start_msg_idx;

        if (consumer_msg_count != producer_msg_count) {
            std::cout << "error" << std::endl;
            std::cout << "msg_count " << msg_count << ", consumer_msg_count is "
                      << consumer_msg_count << ", producer_msg_count is " << producer_msg_count
                      << std::endl;
            // return;
        }

        auto printData = [](std::string tip, uint64_t msg_count, uint64_t total_ns) {
            double avg_ns_per_msg = static_cast<double>(total_ns) / msg_count;
            double msgs_per_sec = 1e9 / avg_ns_per_msg;
            std::cout << std::format("{} : msg_count {}, total_ns {}, {:.2f} ns/msg, {} msgs/s\n",
                                     tip, formatNum(msg_count), total_ns, avg_ns_per_msg,
                                     formatNum(msgs_per_sec));
        };

        printData("生产", msg_count, producer_ns);
        printData("消费", msg_count, consumer_ns);

        std::cout << "push_failed.  " << ", buffer_is_full "
                  << formatNum(err_count[push_buff_is_full]) << std::endl;
        std::cout << "pop_failed.  buffer_is_empty " << formatNum(err_count[pop_buff_is_empty]) << std::endl;
    }
};

} // namespace wlogger
} // namespace wtool