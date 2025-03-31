#include "client.hpp"
#include "conn/cpphttplibconnector.hpp"
#include "conn/inmemoryconnector.hpp"
#include "server.hpp"

#include <iostream>
#include <jsonrpccxx/client.hpp>
#include <jsonrpccxx/server.hpp>
#include <vector>

#include <log/loguru.hpp>

using namespace jsonrpccxx;
using namespace std;

int main(int argc, char *argv[])
{
    loguru::g_preamble_date = false;    // 是否显示日期
    loguru::g_preamble_time = true;     // 是否显示时间
    loguru::g_preamble_uptime = true;   // 是否显示运行时
    loguru::g_preamble_thread = true;   // 是否显示线程
    loguru::g_preamble_file = false;    // 是否显示文件
    loguru::g_preamble_verbose = false; // 是否显示级别
    loguru::g_preamble_pipe = false;    // 是否加上管道符|
    loguru::init(argc, argv);

    // 设置日志文件
    loguru::add_file("logfile.txt", loguru::Append, loguru::Verbosity_INFO);
    LOG_F(INFO, "Running http example");

    AppServer ser;

    std::this_thread::sleep_for(1s);

    AppClient cli;
    cli.test();

    std::this_thread::sleep_for(10s);
    return 0;
}
