import socket
import time
import argparse
import threading

def get_local_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # 这里连接谷歌 DNS，实际上不会发包，只是系统选最优网卡
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
    except Exception:
        ip = "127.0.0.1"
    finally:
        s.close()
    return ip

def parse_args():
    parser = argparse.ArgumentParser(description="TCP client sends message every 1ms and receives messages.")
    parser.add_argument('--host', type=str, default=None, help="Server IP to connect to (default: auto detect)")
    parser.add_argument('--port', type=int, default=2345, help="Server port (default 2345)")
    return parser.parse_args()

def send_loop(sock):
    counter = 0
    while True:
        message = f"Hello {counter}\n".encode('utf-8')
        try:
            sock.sendall(message)
        except Exception as e:
            print(f"[Send error] {e}")
            break
        counter += 1
        time.sleep(0.001)

def recv_loop(sock):
    while True:
        try:
            data = sock.recv(1024)
            if not data:
                print("[Connection closed by server]")
                break
            print(f"[From server]: {data.decode(errors='ignore').strip()}")
        except Exception as e:
            print(f"[Receive error] {e}")
            break

def main():
    args = parse_args()
    host = args.host if args.host else get_local_ip()
    port = args.port

    print(f"Connecting to {host}:{port} ...")

    with socket.create_connection((host, port)) as s:
        # 启动发送和接收线程
        t_send = threading.Thread(target=send_loop, args=(s,), daemon=True)
        t_recv = threading.Thread(target=recv_loop, args=(s,), daemon=True)
        t_send.start()
        t_recv.start()

        # 等待两个线程结束（通常是连接断开）
        t_send.join()
        t_recv.join()

if __name__ == "__main__":
    main()
