#pragma once
#include <nlohmann/json.hpp>

#include <chrono>
#include <mutex>
#include <thread>

using namespace std::chrono_literals;

enum class category { order, cash_carry };

struct Product {
public:
    std::string id;
    double price;
    std::string name;
    category cat;
};

NLOHMANN_JSON_SERIALIZE_ENUM(category, {{category::order, "order"}, {category::cash_carry, "cc"}})

inline void to_json(nlohmann::json &j, const Product &p)
{
    j = nlohmann::json {{"id", p.id}, {"price", p.price}, {"name", p.name}, {"category", p.cat}};
}

inline void from_json(const nlohmann::json &j, Product &p)
{
    j.at("name").get_to(p.name);
    j.at("id").get_to(p.id);
    j.at("price").get_to(p.price);
    j.at("category").get_to(p.cat);
}

int g_msgid()
{
    static int id = 0;
    static std::mutex mtx; 
    std::lock_guard<std::mutex> lock(mtx);

    id++;
    return id;
}

class EventListener
{
public:
    std::string name;
    std::string handleName;
    int onMousePressed(int x, int y)
    {
        std::cout << "mouseEnter 1 : " << x << y << "\n";
        std::this_thread::sleep_for(3s);
        std::cout << "mouseEnter 2 : " << x << y << "\n";
        return 10;
    }
    void onMouseRelease(int x, int y)
    {
        std::cout << "onMouseRelease 1: " << x << y << "\n";
        std::this_thread::sleep_for(0.2s);
        std::cout << "onMouseRelease 2: " << x << y << "\n";
    }
};

void to_json(nlohmann::json &j, const EventListener &p)
{
    j = nlohmann::json {{"name", p.name}, {"handleName", p.handleName}};
}

void from_json(const nlohmann::json &j, EventListener &p)
{
    j.at("name").get_to(p.name);
    j.at("handleName").get_to(p.handleName);
}
