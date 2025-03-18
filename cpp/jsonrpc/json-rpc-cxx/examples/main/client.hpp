#pragma once
#include <jsonrpccxx/client.hpp>
#include <map>
#include <string>
#include <vector>
#include "conn/cpphttplibconnector.hpp"
#include "conn/inmemoryconnector.hpp"
#include "conn/asiotcpconnector.hpp"
#include "types.h"

using namespace jsonrpccxx;

class AppClient
{
public:
    explicit AppClient()
    {
        httpClient = new AsioClientConnector("localhost", 8484);
        client = new JsonRpcClient(*httpClient, version::v2);
    }
    void test()
    {
        Product p = {"0xff", 22.4, "Product hello", category::cash_carry};
        C_AddProduct(p) ;
        std::cout << "Adding product: " << std::boolalpha << p.id << "\n";

        p.id = "0xff2";
        p.name = "Product world";
        C_AddProduct(p);
        std::cout << "Adding product: " << std::boolalpha <<  p.id << "\n";

        auto all = C_AllProducts();
        for (const auto &p : all) {
            std::cout << p.name << "\n";
        }

        std::cout << "\n";
        int ret = calc(5, 6);
        std::cout << ret << "\n";
    }

    bool C_AddProduct(const Product &p)
    {
        return client->CallMethod<bool>(1, "AddProduct", {p});
    }
    Product C_GetProduct(const std::string &id)
    {
        return client->CallMethod<Product>(1, "GetProduct", {id});
    }
    std::vector<Product> C_AllProducts()
    {
        return client->CallMethod<std::vector<Product>>(1, "AllProducts", {});
    }

    int calc(int a, int b)
    {
        return client->CallMethod<int>(1, "calc", {a, b});
    }

private:
    JsonRpcClient *client;

    AsioClientConnector *httpClient;
};
