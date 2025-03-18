#include "client.hpp"
#include "conn/cpphttplibconnector.hpp"
#include "conn/inmemoryconnector.hpp"
#include "server.hpp"

#include <iostream>
#include <jsonrpccxx/client.hpp>
#include <jsonrpccxx/server.hpp>
#include <vector>

using namespace jsonrpccxx;
using namespace std;


int main()
{
    cout << "Running http example"
         << "\n";
    AppServer ser;

    std::this_thread::sleep_for(1s);

    AppClient cli;
    cli.test();

    std::this_thread::sleep_for(2s);
    return 0;
}
