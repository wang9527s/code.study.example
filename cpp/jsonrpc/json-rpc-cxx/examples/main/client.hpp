#pragma once
#include <jsonrpccxx/client.hpp>
#include <map>
#include <string>
#include <vector>
#include "conn/cpphttplibconnector.hpp"
#include "conn/inmemoryconnector.hpp"
#include "types.h"

using namespace jsonrpccxx;

class AppClient
{
public:
    explicit AppClient()
    {
        httpClient = new CppHttpLibClientConnector("localhost", 8484);
        client = new JsonRpcClient(*httpClient, version::v2);
    }
    void test()
    {
        Product p = {"0xff", 22.4, "Product hello", category::cash_carry};
        std::cout << "Adding product: " << std::boolalpha << C_AddProduct(p) << "\n";

        p.id = "0xff2";
        p.name = "Product world";
        std::cout << "Adding product: " << std::boolalpha << C_AddProduct(p) << "\n";

        auto all = C_AllProducts();
        for (const auto &p : all) {
            std::cout << p.name << "\n";
        }

        std::cout << "\n";
        std::cout << calc(5, 6) << "\n";
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

    CppHttpLibClientConnector *httpClient;
};
