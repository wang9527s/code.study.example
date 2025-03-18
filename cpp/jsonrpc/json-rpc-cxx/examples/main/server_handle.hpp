#pragma once
#include <map>
#include <string>
#include <vector>
#include "types.h"
#include <thread>

class ServerHandle {
public:
  ServerHandle() :
    products() {}

    bool AddProduct(const Product &p) {
      if (products.find(p.id) != products.end())
        return false;
      products[p.id] = p;
      return true;
    }
    
    const Product& GetProduct(const std::string &id) {
      if (products.find(id) == products.end())
        return Product();
      return products[id];
    }
    std::vector<Product> AllProducts() {
      std::vector<Product> result;
      for (const auto &p : products)
        result.push_back(p.second);
      return result;
    }
    int calc(int a, int b) {
      return a + b;
    }
    void registerEvent(ClientEvent * event) { 
       std::thread t([event]{
        std::this_thread::sleep_for(std::chrono::seconds(1));
        event->mouseEnter(3,3);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        event->mouseLeave(9,9);
       });
       t.detach();
    }

private:
  std::map<std::string, Product> products;
};

