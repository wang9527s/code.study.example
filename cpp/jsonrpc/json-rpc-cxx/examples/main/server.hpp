#pragma once
#include <map>
#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include "types.h"

#include "conn/asiotcpconnector.hpp"

using namespace jsonrpccxx;

class ServerHandle
{
public:
    ServerHandle()
        : products()
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

private:
    std::map<std::string, Product> products;
};

class AppServer
{
public:
    AppServer()
    {
        rpcServer.Add("GetProduct", GetHandle(&ServerHandle::GetProduct, app), {"id"});
        rpcServer.Add("AddProduct", GetHandle(&ServerHandle::AddProduct, app), {"product"});
        rpcServer.Add("AllProducts", GetHandle(&ServerHandle::AllProducts, app), {});
        rpcServer.Add("calc", GetHandle(&ServerHandle::calc, app), {"int", "int"});

        httpServer = new AsioServerConnector(rpcServer, 8484);
        httpServer->StartListening();
        std::cout << "Starting http server: "  << "\n";
    }
    JsonRpc2Server rpcServer;

    // Bindings
    ServerHandle app;
    AsioServerConnector *httpServer;
};
