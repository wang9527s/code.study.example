#pragma once
#include <string>

namespace jsonrpccxx {
    class IClientConnector {
    public:
        virtual ~IClientConnector() = default;
        virtual std::string Send(const std::string &request) = 0;
        virtual void Notify(const std::string &request) = 0;
    };
}