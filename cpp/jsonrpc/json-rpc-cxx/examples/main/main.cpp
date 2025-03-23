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

int main(int argc, char* argv[])
{
    cout << "Running http example"
         << "\n";

    loguru::init(argc, argv);
    loguru::g_preamble_date      = false;  // 是否显示日期
    loguru::g_preamble_time      = true;  // 是否显示时间
    loguru::g_preamble_uptime    = true;  // 是否显示运行时
    loguru::g_preamble_thread    = true;  // 是否显示线程
    loguru::g_preamble_file      = false;  // 是否显示文件
    loguru::g_preamble_verbose   = false;  // 是否显示级别
    loguru::g_preamble_pipe      = false;  // 是否加上管道符|

    // 设置日志文件
    loguru::add_file("logfile.txt", loguru::Append, loguru::Verbosity_INFO);

    // 日志输出
    LOG_F(INFO, "This is an info message.");
    LOG_F(WARNING, "This is a warning message.");
    LOG_F(ERROR, "This is an error message.");


    AppServer ser;

    std::this_thread::sleep_for(1s);

    AppClient cli;
    cli.test();

    std::this_thread::sleep_for(25s);
    return 0;
}
