#include "conn/inmemoryconnector.hpp"
#include "conn/cpphttplibconnector.hpp"
#include "server.hpp"
#include "client.hpp"

#include <iostream>
#include <vector>
#include <jsonrpccxx/client.hpp>
#include <jsonrpccxx/server.hpp>

using namespace jsonrpccxx;
using namespace std;

ClientEvent cEvent;


int main() {
  cout << "Running http example" << "\n";
  AppServer ser;
  
  std::this_thread::sleep_for(1s);
  
  AppClient cli;
  cli.test();

  std::this_thread::sleep_for(2s);
  return 0;
}
