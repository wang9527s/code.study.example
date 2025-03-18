#pragma once
#include <map>
#include <string>
#include <vector>
#include "types.h"

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

private:
  std::map<std::string, Product> products;
};

