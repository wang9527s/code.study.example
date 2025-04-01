
### 基于jsonrpc的demo

支持普通函数，以及回调函数的注册。  

原本的demo，只适用于普通函数。
想要支持回调的注册，需要在client端注册函数(Add)，server端在合适的时候进行rpc，和原本的逻辑相反[](https://github.com/palacaze/sigslot/blob/master/example/connection.cpp)

1. 双端都可以主动发起调用，即支持主动request请求、被动response回复。
   同一个socket有多处收发地方，注意竞争条件。我使用了**waiting_response**进行逻辑判断
2. demo，server只支持单连接
3. 似乎[async-sockets-cpp](https://github.com/eminfedar/async-sockets-cpp) + [sigslot::signal<>](https://github.com/palacaze/sigslot/blob/master/example/connection.cpp)是一个更好的方案，asio太重了