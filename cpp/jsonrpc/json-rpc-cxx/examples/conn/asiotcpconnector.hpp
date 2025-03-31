#pragma once

#define ASIO_STANDALONE
#include <asio/asio.hpp>
#include <atomic>
#include <condition_variable>
#include <jsonrpccxx/iclientconnector.hpp>
#include <jsonrpccxx/server.hpp>
#include <log/loguru.hpp>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>

class AsioClientConnector : public jsonrpccxx::IClientConnector
{
public:
    AsioClientConnector(jsonrpccxx::JsonRpcServer &json_rpc_cb_server_, const std::string &host, int port)
        : rpc_cb_server_(json_rpc_cb_server_)
        , io_context()
        , socket(io_context)
        , has_response(false)
        , waiting_response(false)
    {
        asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(host, std::to_string(port));
        asio::connect(socket, endpoints);

        listener_thread = std::thread([this]() {
            try {
                while (true) {
                    uint32_t len = 0;
                    asio::read(socket, asio::buffer(&len, sizeof(len)));
                    len = ntohl(len);
                    std::string message(len, '\0');
                    asio::read(socket, asio::buffer(message.data(), message.size()));

                    LOG_F(INFO, "[Client Listener] Received message: %s", message.c_str());

                    std::lock_guard<std::mutex> lock(response_mutex);
                    if (waiting_response) {
                        last_response = message;
                        has_response = true;
                        waiting_response = false;
                        response_cv.notify_one();
                    }
                    else {
                        std::string response = rpc_cb_server_.HandleRequest(message);
                        if (!response.empty()) {
                            LOG_F(INFO, "[Client Listener] Callback response: %s", response.c_str());
                            uint32_t resplen = htonl(static_cast<uint32_t>(response.size()));
                            asio::write(socket, asio::buffer(&resplen, sizeof(resplen)));
                            asio::write(socket, asio::buffer(response));
                        }
                        else {
                            LOG_F(INFO, "[Client Listener] Callback response: (empty)");
                        }
                    }
                }
            }
            catch (const std::exception &e) {
                LOG_F(ERROR, "[Client Listener] Exception: %s", e.what());
            }
        });
    }

    ~AsioClientConnector()
    {
        try {
            socket.close();
        }
        catch (...) {
        }
        if (listener_thread.joinable())
            listener_thread.join();
    }

    std::string Send(const std::string &request) override
    {
        std::lock_guard<std::mutex> send_lock(send_mutex);
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

        return last_response;
    }

private:
    jsonrpccxx::JsonRpcServer &rpc_cb_server_;
    asio::io_context io_context;
    asio::ip::tcp::socket socket;
    std::thread listener_thread;

    std::mutex send_mutex;
    std::mutex response_mutex;
    std::condition_variable response_cv;

    std::string last_response;
    bool has_response;
    bool waiting_response;
};

class AsioServerConnector : public jsonrpccxx::IClientConnector
{
public:
    AsioServerConnector(jsonrpccxx::JsonRpcServer &rpc_cb_server_, int port)
        : rpc_cb_server_(rpc_cb_server_)
        , io_context()
        , acceptor(io_context)
        , port(port)
        , is_running(false)
    {
    }

    ~AsioServerConnector()
    {
        Stop();
    }

    void StartListening()
    {
        if (is_running)
            return;
        is_running = true;

        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
        acceptor.open(endpoint.protocol());
        acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
        acceptor.bind(endpoint);
        acceptor.listen();

        server_thread = std::thread([this]() { AcceptLoop(); });
    }

    void Stop()
    {
        is_running = false;
        try {
            acceptor.close();
        }
        catch (...) {
        }
        io_context.stop();
        if (server_thread.joinable())
            server_thread.join();
    }

    std::string Send(const std::string &request) override
    {
        std::lock_guard<std::mutex> lock(send_mutex);
        if (!latest_socket)
            throw std::runtime_error("No active connection");

        {
            std::lock_guard<std::mutex> lock(response_mutex);
            waiting_response = true;
            has_response = false;
        }

        uint32_t len = htonl(static_cast<uint32_t>(request.size()));
        asio::write(*latest_socket, asio::buffer(&len, sizeof(len)));
        asio::write(*latest_socket, asio::buffer(request));

        std::unique_lock<std::mutex> response_lock(response_mutex);
        response_cv.wait(response_lock, [this]() { return has_response; });

        return last_response;
    }

private:
    void AcceptLoop()
    {
        while (is_running) {
            auto socket = std::make_shared<asio::ip::tcp::socket>(io_context);
            acceptor.accept(*socket);

            {
                std::lock_guard<std::mutex> lock(conn_mutex);
                latest_socket = socket;
            }

            std::thread([this, socket]() {
                try {
                    while (true) {
                        uint32_t len = 0;
                        asio::read(*socket, asio::buffer(&len, sizeof(len)));
                        len = ntohl(len);
                        std::string message(len, '\0');
                        asio::read(*socket, asio::buffer(message.data(), message.size()));
                        LOG_F(INFO, "[Server Listener] Received message: %s", message.c_str());

                        std::lock_guard<std::mutex> lock(response_mutex);
                        if (waiting_response) {
                            last_response = message;
                            has_response = true;
                            waiting_response = false;
                            response_cv.notify_one();
                        }
                        else {
                            std::string response = rpc_cb_server_.HandleRequest(message);
                            if (!response.empty()) {
                                LOG_F(INFO, "[Server Listener] Response: %s", response.c_str());
                                uint32_t resplen = htonl(static_cast<uint32_t>(response.size()));
                                asio::write(*socket, asio::buffer(&resplen, sizeof(resplen)));
                                asio::write(*socket, asio::buffer(response));
                            }
                            else {
                                LOG_F(INFO, "[Server Listener] Empty response");
                            }
                        }
                    }
                }
                catch (const std::exception &e) {
                    LOG_F(ERROR, "[Server Listener] Exception: %s", e.what());
                }
            }).detach();
        }
    }

    jsonrpccxx::JsonRpcServer &rpc_cb_server_;
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor;
    std::shared_ptr<asio::ip::tcp::socket> latest_socket;
    std::mutex conn_mutex;
    std::thread server_thread;
    std::atomic<bool> is_running;

    std::mutex send_mutex;
    std::mutex response_mutex;
    std::condition_variable response_cv;

    std::string last_response;
    bool has_response = false;
    bool waiting_response = false;
    int port;
};
