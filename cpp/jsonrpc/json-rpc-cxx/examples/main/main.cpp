#include "conn/inmemoryconnector.hpp"
#include "conn/cpphttplibconnector.hpp"
#include "server_handle.hpp"
#include "client_handle.hpp"

#include <iostream>
#include <vector>
#include <jsonrpccxx/client.hpp>
#include <jsonrpccxx/server.hpp>

using namespace jsonrpccxx;
using namespace std;

ClientEvent cEvent;

void doWarehouseStuff(IClientConnector &clientConnector) {
  JsonRpcClient client(clientConnector, version::v2);
  ClientHandle appClient(client);
  Product p;
  p.id = "0xff";
  p.price = 22.4;
  p.name = "Product hello";
  p.cat = category::cash_carry;
  cout << "Adding product: " << std::boolalpha << appClient.C_AddProduct(p) << "\n";

  p.id = "0xff2";
  p.name = "Product world";
  cout << "Adding product: " << std::boolalpha << appClient.C_AddProduct(p) << "\n";

  Product p2 = appClient.C_GetProduct("0xff");
  cout << "Found product: " << p2.name << "\n";
  try {
    appClient.C_GetProduct("0xff2");
  } catch (JsonRpcException &e) {
    cerr << "Error finding product: " << e.what() << "\n";
  }

  auto all = appClient.C_AllProducts();
  for (const auto &p: all) {
    cout << p.name << endl;
  }

  cout << "\n\n";
  cout << appClient.calc(5, 6) << "\n";

  appClient.registerEvent(&cEvent);
}

int main() {
  JsonRpc2Server rpcServer;

  // Bindings
  ServerHandle app;
  rpcServer.Add("GetProduct", GetHandle(&ServerHandle::GetProduct, app), {"id"});
  rpcServer.Add("AddProduct", GetHandle(&ServerHandle::AddProduct, app), {"product"});
  rpcServer.Add("AllProducts", GetHandle(&ServerHandle::AllProducts, app), {});
  rpcServer.Add("calc", GetHandle(&ServerHandle::calc, app), {"int", "int"});

  rpcServer.Add("registerEvent.mouseEnter", GetHandle(&ServerHandle::calc, app), {"int", "int"});

  // cout << "Running in-memory example" << "\n";
  // InMemoryConnector inMemoryConnector(rpcServer);
  // doWarehouseStuff(inMemoryConnector);

  // std::this_thread::sleep_for(1s);
  // cout << "\n" << "\n";

  cout << "Running http example" << "\n";
  CppHttpLibServerConnector httpServer(rpcServer, 8484);
  cout << "Starting http server: " << std::boolalpha << httpServer.StartListening() << "\n";
  CppHttpLibClientConnector httpClient("localhost", 8484);
  std::this_thread::sleep_for(1s);
  doWarehouseStuff(httpClient);

  return 0;
}
