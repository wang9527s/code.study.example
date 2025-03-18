#pragma once
#include <map>
#include <string>
#include <vector>
#include "types.h"
#include <jsonrpccxx/client.hpp>

using namespace jsonrpccxx;

class ClientEvent_Porxy {
  public:
    void mouseEnter(int x, int y) {
      if (cb != nullptr) {
        cb->mouseEnter(x, y);
      }
      std::cout << "proxy mouseEnter: " << x << y << "\n";
    }
    void mouseLeave(int x, int y) {
      if (cb != nullptr) {
        cb->mouseLeave(x, y);
      }
      std::cout << "proxy mouseLeave: " << x << y << "\n";
    }
    ClientEvent * cb = nullptr;
  };

  using JsonRpc2ServerPtr = std::shared_ptr<JsonRpc2Server>;

class ClientHandle {
  public:
    explicit ClientHandle(JsonRpcClient &client) : client(client) {
      json_rpc_cb_server_ = std::make_shared<JsonRpc2Server>();
    }

    bool C_AddProduct(const Product &p) { return client.CallMethod<bool>(1, "AddProduct", {p}); }
    Product C_GetProduct(const std::string &id) { return client.CallMethod<Product>(1, "GetProduct", {id}); }
    std::vector<Product> C_AllProducts() { return client.CallMethod<std::vector<Product>>(1, "AllProducts", {}); }
  
    int calc(int a, int b) {return client.CallMethod<int>(1, "calc", {a, b}); }
    void registerEvent(ClientEvent * event) { 

      json_rpc_cb_server_->Add("registerEvent.mouseEnter", GetHandle(&ClientEvent_Porxy::mouseEnter, event_proxy), {"int", "int"});
      json_rpc_cb_server_->Add("registerEvent.mouseLeave", GetHandle(&ClientEvent_Porxy::mouseLeave, event_proxy), {"int", "int"});

      // client.CallMethod<void>(1, "registerEvent", {});
    }
  private:
    JsonRpcClient &client;
    ClientEvent_Porxy event_proxy;

    JsonRpc2ServerPtr json_rpc_cb_server_ = nullptr;
    
  
  };

