#pragma once
#include <jsonrpccxx/client.hpp>
#include <jsonrpccxx/server.hpp>
#include <map>
#include <string>
#include <vector>
#include "conn/asiotcpconnector.hpp"
#include "conn/cpphttplibconnector.hpp"
#include "conn/inmemoryconnector.hpp"
#include "types.h"

using namespace jsonrpccxx;

class AppClient
{
public:
    EventListener evt;
    explicit AppClient()
    {
        evt.handleName = "EventListener";
        evt.name = "123";
        json_rpc_cb_server_ = new JsonRpc2Server;
        httpClient = new AsioClientConnector(*json_rpc_cb_server_, "localhost", 8484);
        client = new JsonRpcClient(*httpClient, version::v2);
    }
    void test()
    {
        Product p = {"0xff", 22.4, "Product hello", category::cash_carry};
        C_AddProduct(p);
        std::cout << "Adding product: " << std::boolalpha << p.id << "\n";

        p.id = "0xff2";
        p.name = "Product world";
        C_AddProduct(p);
        std::cout << "Adding product: " << std::boolalpha << p.id << "\n";

        auto all = C_AllProducts();
        for (const auto &p : all) {
            std::cout << p.name << "\n";
        }

        std::cout << "\n";
        int ret = calc(5, 6);
        std::cout << ret << "\n";
        std::string key = evt.handleName + "@" + evt.name;
        key = "onMousePressed";

        bool res = json_rpc_cb_server_->Add(key, GetHandle(&EventListener::onMousePressed, evt), {"x", "y"});
        std::cout << "add:  " << key << " " << res << std::to_string((long long)json_rpc_cb_server_) << "\n";

        res = json_rpc_cb_server_->Add(key, GetHandle(&EventListener::onMouseRelease, evt), {"x", "y"});
        std::cout << "add:  " << key << " " << res << std::to_string((long long)json_rpc_cb_server_) << "\n";

        client->CallMethod<int>(g_msgid(), "registerEventListener", {evt});
    }

    bool C_AddProduct(const Product &p)
    {
        return client->CallMethod<bool>(g_msgid(), "AddProduct", {p});
    }
    Product C_GetProduct(const std::string &id)
    {
        return client->CallMethod<Product>(g_msgid(), "GetProduct", {id});
    }
    std::vector<Product> C_AllProducts()
    {
        return client->CallMethod<std::vector<Product>>(g_msgid(), "AllProducts", {});
    }

    int calc(int a, int b)
    {
        return client->CallMethod<int>(g_msgid(), "calc", {a, b});
    }

private:
    JsonRpcClient *client;
    JsonRpc2Server *json_rpc_cb_server_;

    AsioClientConnector *httpClient;
};
