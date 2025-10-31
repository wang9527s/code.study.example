
实际项目中，Client调用Server接口，Server也会触发Client的接口。  
我们用的是：  
1. C调用S的普通接口。  
2. C注册一个lister（class回调）给S，然后S在合适的时机触发。

### Client调用Server接口

首先，自定义的数据结构需要能序列化和反序列化为json

```cpp
struct Product {
public:
    std::string id;
    double price;
    std::string name;
};

inline void to_json(nlohmann::json &j, const Product &p)
{
    j = nlohmann::json {{"id", p.id}, {"price", p.price}, {"name", p.name}};
}

inline void from_json(const nlohmann::json &j, Product &p)
{
    j.at("name").get_to(p.name);
    j.at("id").get_to(p.id);
    j.at("price").get_to(p.price);
}

```

#### Server

```cpp
class ServerHandle
{
    bool AddProduct(const Product &p) {}
	const Product &GetProduct(const std::string &id) {}
}
class TransportServer
{
	void recvJson(json request) {
		server.HandleRequest(request);
	}
private:
	jsonrpccxx::JsonRpcServer &server;
}

//JsonRpc2Server: public JsonRpcServer
JsonRpc2Server rpcServer;
rpcServer.Add("GetProduct", GetHandle(&ServerHandle::GetProduct, app), {"id"});
rpcServer.Add("AddProduct", GetHandle(&ServerHandle::AddProduct, app), {"product"});
TransportServer *trans = new TransportServer(rpcServer, 8484);
```

如上  
1. 调用JsonRpc2Server::Add，注册ServerHandle的接口到jsonrpc框架的map中。两个参数分别是字符串key和函数指针value。  
2. 传递调用JsonRpc2Server实例给transport对象，然后在接收到json请求的时候，调用JsonRpc2Server::HandleRequest接口。  
根据请求数据，调用1中注册的接口。

#### Client

```cpp
class TransportClient : public jsonrpccxx::IClientConnector
{
    std::string Send(const std::string &request) override {}
};

trans = new TransportClient("localhost", 8484);
client = new JsonRpcClient(*trans, version::v2);

// 接口调用
client->CallMethod<Product>(1, "GetProduct", {id});
```

如上  
1. Client中的transport需要集成自IClientConnector，并实现Send接口，该接口需要发送request请求到Server。  
注意，如果出现错误，可以抛出jsonrpc框架自带的异常或者自己返回一个json数据表示出错。  
2. JsonRpcClient构造的时候，需要传递trans对象，然后使用JsonRpcClient::CallMethod等接口即可。（框架会使用刚刚传递trans对象，传递数据给Server）

### Client注册回调到Server

  可以反方向理解，Server中包含了Client的功能，Client中包含了Server的功能