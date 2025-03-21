#pragma once

#define ASIO_STANDALONE
#include <asio/asio.hpp>
#include <jsonrpccxx/iclientconnector.hpp>
#include <string>
#include <sstream>
#include <jsonrpccxx/server.hpp>
#include <thread>
#include <atomic>
#include <unordered_set>
#include <memory>
#include <mutex>

// client 端继承自 IClientConnector, JsonRpcClient持有IClientConnector指针
//   用于给服务端发消息，基类中新增 Notify 函数，
class AsioClientConnector : public jsonrpccxx::IClientConnector {
public:
    AsioClientConnector(const std::string &host, int port)
        : io_context(), socket(io_context) {
        asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(host, std::to_string(port));
        asio::connect(socket, endpoints);
    }

    std::string Send(const std::string &request) override {
        std::cout << "client, send " << request << "\n";
        uint32_t len = htonl(static_cast<uint32_t>(request.size()));
        asio::write(socket, asio::buffer(&len, sizeof(len)));
        asio::write(socket, asio::buffer(request));

        uint32_t response_len;
        asio::read(socket, asio::buffer(&response_len, sizeof(response_len)));
        response_len = ntohl(response_len);

        std::string response(response_len, '\0');
        asio::read(socket, asio::buffer(response.data(), response.size()));
        return response;
    }


private:
    asio::io_context io_context;
    asio::ip::tcp::socket socket;
};


class AsioServerConnector : public jsonrpccxx::IClientConnector {
public:
    AsioServerConnector(jsonrpccxx::JsonRpcServer &server, int port)
        : server(server), io_context(), acceptor(io_context), port(port), is_running(false) {}

    ~AsioServerConnector() { StopListening(); }

    void StartListening() {
        if (is_running) return;
        is_running = true;

        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
        acceptor.open(endpoint.protocol());
        acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
        acceptor.bind(endpoint);
        acceptor.listen();

        server_thread = std::thread([this]() { Run(); });
    }

    void StopListening() {
        is_running = false;
        io_context.stop();
        if (server_thread.joinable()) server_thread.join();
    }

    // TODO 目前仅支持单连接
    std::string Send(const std::string &request) override {
        std::cout << "server, send  444444444444444444" << request << "\n";
        uint32_t len = htonl(static_cast<uint32_t>(request.size()));
        asio::write(*latest_socket_, asio::buffer(&len, sizeof(len)));
        asio::write(*latest_socket_, asio::buffer(request));

        uint32_t response_len;
        asio::read(*latest_socket_, asio::buffer(&response_len, sizeof(response_len)));
        response_len = ntohl(response_len);

        std::string response(response_len, '\0');
        asio::read(*latest_socket_, asio::buffer(response.data(), response.size()));
        return response;
    }

private:
    void Run() {
        while (is_running) {
            std::shared_ptr<asio::ip::tcp::socket> socket = std::make_shared<asio::ip::tcp::socket>(io_context);
            std::error_code ec;
            acceptor.accept(*socket, ec);
            if (ec) continue;

            {
                std::lock_guard<std::mutex> lock(conn_mutex);
                active_connections.insert(socket);
            }

            latest_socket_ = socket;
            std::thread([this, socket]() {
                try {
                    while (true) {
                        uint32_t len = 0;
                        std::size_t read_len = asio::read(*socket, asio::buffer(&len, sizeof(len)));
                        if (read_len == 0) break; // connection closed
                        len = ntohl(len);

                        std::string request(len, '\0');
                        asio::read(*socket, asio::buffer(request.data(), request.size()));

                        std::cout << "server, recv" << request << "\n";
                        std::string response = server.HandleRequest(request);
                        std::cout << "server, response" << response << "\n";

                        uint32_t resplen = htonl(static_cast<uint32_t>(response.size()));
                        asio::write(*socket, asio::buffer(&resplen, sizeof(resplen)));
                        asio::write(*socket, asio::buffer(response));
                    }
                } catch (const std::exception &e) {
                    // 可以添加日志输出：std::cerr << "Client connection error: " << e.what() << std::endl;
                }

                std::lock_guard<std::mutex> lock(conn_mutex);
                active_connections.erase(socket);
            }).detach();
        }
    }

    std::shared_ptr<asio::ip::tcp::socket> latest_socket_;
    jsonrpccxx::JsonRpcServer &server;
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor;
    std::unordered_set<std::shared_ptr<asio::ip::tcp::socket>> active_connections;
    std::mutex conn_mutex;
    std::thread server_thread;
    std::atomic<bool> is_running;
    int port;
};
