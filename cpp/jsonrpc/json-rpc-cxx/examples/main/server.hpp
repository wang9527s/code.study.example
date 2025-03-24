#pragma once
#include <chrono>
#include <iostream>
#include <jsonrpccxx/client.hpp>
#include <jsonrpccxx/server.hpp>
#include <map>
#include <string>
#include <thread>
#include <vector>
#include "conn/asiotcpconnector.hpp"
#include "log/loguru.hpp"
#include "types.h"

using namespace jsonrpccxx;
using namespace std;

class BaseServer
{
public:
    virtual void regiserCb(EventListener evt) = 0;
};

class ServerHandle
{
public:
    BaseServer *server_;
    ServerHandle(BaseServer *server)
        : products()
        , server_(server)
    {
    }

    bool AddProduct(const Product &p)
    {
        if (products.find(p.id) != products.end())
            return false;
        products[p.id] = p;
        return true;
    }

    const Product &GetProduct(const std::string &id)
    {
        if (products.find(id) == products.end())
            return Product();
        return products[id];
    }
    std::vector<Product> AllProducts()
    {
        std::vector<Product> result;
        for (const auto &p : products)
            result.push_back(p.second);
        return result;
    }
    int calc(int a, int b)
    {
        return a + b;
    }
    int registerEventListener(EventListener evt)
    {
        std::cout << "sever registerEventListener" << std::endl;
        server_->regiserCb(evt);

        return 0;
    }

private:
    std::map<std::string, Product> products;
};

class AppServer : public BaseServer
{
public:
    AppServer()
    {
        app = new ServerHandle(this);
        rpcServer.Add("GetProduct", GetHandle(&ServerHandle::GetProduct, *app), {"id"});
        rpcServer.Add("AddProduct", GetHandle(&ServerHandle::AddProduct, *app), {"product"});
        rpcServer.Add("AllProducts", GetHandle(&ServerHandle::AllProducts, *app), {});
        rpcServer.Add("calc", GetHandle(&ServerHandle::calc, *app), {"int", "int"});

        rpcServer.Add("registerEventListener", GetHandle(&ServerHandle::registerEventListener, *app), {"evt"});

        httpServer = new AsioServerConnector(rpcServer, 8484);
        httpServer->StartListening();

        json_rpc_cb_client_ = new JsonRpcClient(*httpServer, version::v2);
        // json_rpc_cb_client_->CallMethod<int>(g_msgid(), "registerEventListener", {evt});
    }
    void regiserCb(EventListener evt)
    {
        std::thread([this, evt]() {
            std::this_thread::sleep_for(2s);
            std::string key;
            key = "onMousePressed";
            int aa = json_rpc_cb_client_->CallMethod<int>(g_msgid(), key, {100, 100});
            LOG_F(INFO, "[Server ] call cb , res = %d", aa);
        }).detach();
        std::thread([this, evt]() {
            std::string key;
            std::this_thread::sleep_for(2.5s);
            key = "onMouseRelease";
            json_rpc_cb_client_->CallNotification(key, {100, 99});
        }).detach();
    }
    JsonRpc2Server rpcServer;
    JsonRpcClient *json_rpc_cb_client_ = nullptr;

    // Bindings
    ServerHandle *app;
    AsioServerConnector *httpServer;
};
