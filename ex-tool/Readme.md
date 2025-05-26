
**tcp/start_client.py**

使用指定端口和本机ip连接server，连接成功后，循环发送数据

```
python3 start_client.py --port=2345
```

**tcp/start_server.py**

监听指定端口的连接，连接成功后，循环发送数据

```
python3 start_server.py --port=2345
```