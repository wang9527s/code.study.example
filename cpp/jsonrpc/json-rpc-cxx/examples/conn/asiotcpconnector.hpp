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



class AsioClientConnector : public jsonrpccxx::IClientConnector {
public:
    AsioClientConnector(jsonrpccxx::JsonRpcServer &json_rpc_cb_server_, const std::string &host, int port)
        : rpc_cb_server_(json_rpc_cb_server_), io_context(), socket(io_context), has_response(false) {
        asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(host, std::to_string(port));
        asio::connect(socket, endpoints);

        // 启动唯一接收线程
        listener_thread = std::thread([this]() {
            try {
                while (true) {
                    uint32_t len = 0;
                    asio::read(socket, asio::buffer(&len, sizeof(len)));
                    len = ntohl(len);

                    std::string message(len, '\0');
                    asio::read(socket, asio::buffer(message.data(), message.size()));

                    {
                        std::lock_guard<std::mutex> lock(response_mutex);
                        // 简单判断：如果正在等待响应，我们把消息认为是回应（或者你可以加 request id 判断）
                        if (waiting_response) {
                            last_response = message;
                            has_response = true;
                            waiting_response = false;
                            response_cv.notify_one();
                        } else {
                            // 否则认为是回调请求，走回调处理
                            std::cout << "[listener] recv cb msg: " << message << std::endl;

                            std::string response = rpc_cb_server_.HandleRequest(message);
                            std::cout << "hhh" << response << "\n";
                            if (!response.empty()) {
                                uint32_t resplen = htonl(static_cast<uint32_t>(response.size()));
                                asio::write(socket, asio::buffer(&resplen, sizeof(resplen)));
                                asio::write(socket, asio::buffer(response));
                            }
                        }
                    }
                }
            } catch (const std::exception &e) {
                std::cerr << "[listener error]: " << e.what() << std::endl;
            }
        });
    }

    ~AsioClientConnector() {
        try {
            socket.close();
        } catch (...) {}
        if (listener_thread.joinable()) listener_thread.join();
    }

    std::string Send(const std::string &request) override {
        std::lock_guard<std::mutex> send_lock(send_mutex);  // 串行化写入

        std::cout << "[Client] sending: " << request << std::endl;

        {
            std::lock_guard<std::mutex> lock(response_mutex);
            waiting_response = true;
            has_response = false;
        }

        uint32_t len = htonl(static_cast<uint32_t>(request.size()));
        asio::write(socket, asio::buffer(&len, sizeof(len)));
        asio::write(socket, asio::buffer(request));

        std::unique_lock<std::mutex> response_lock(response_mutex);
        response_cv.wait(response_lock, [this]() { return has_response; });

        std::cout << "[Client] received response: " << last_response << std::endl;
        return last_response;
    }

private:
    jsonrpccxx::JsonRpcServer &rpc_cb_server_;
    asio::io_context io_context;
    asio::ip::tcp::socket socket;

    std::thread listener_thread;

    std::mutex send_mutex; // 串行化写入

    std::mutex response_mutex;
    std::condition_variable response_cv;
    std::string last_response;
    bool has_response = false;
    bool waiting_response = false;
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
        std::cout << "server, send " << request << "\n";
        uint32_t len = htonl(static_cast<uint32_t>(request.size()));
        asio::write(*latest_socket_, asio::buffer(&len, sizeof(len)));
        asio::write(*latest_socket_, asio::buffer(request));

        uint32_t response_len;
        asio::read(*latest_socket_, asio::buffer(&response_len, sizeof(response_len)));
        response_len = ntohl(response_len);

        std::string response(response_len, '\0');
        asio::read(*latest_socket_, asio::buffer(response.data(), response.size()));
        std::cout << "server recv resp: " << response << "\n";
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
