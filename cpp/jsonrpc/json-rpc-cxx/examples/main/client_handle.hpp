#pragma once
#include <map>
#include <string>
#include <vector>
#include "types.h"
#include <jsonrpccxx/client.hpp>

using namespace jsonrpccxx;

class ClientHandle {
  public:
    explicit ClientHandle(JsonRpcClient &client) : client(client) {}

    bool C_AddProduct(const Product &p) { return client.CallMethod<bool>(1, "AddProduct", {p}); }
    Product C_GetProduct(const std::string &id) { return client.CallMethod<Product>(1, "GetProduct", {id}); }
    std::vector<Product> C_AllProducts() { return client.CallMethod<std::vector<Product>>(1, "AllProducts", {}); }
  
    int calc(int a, int b) {return client.CallMethod<int>(1, "calc", {a, b}); }
  
  private:
    JsonRpcClient &client;
  };