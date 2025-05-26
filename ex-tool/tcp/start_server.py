import socket
import threading
import time
import argparse

INTERVAL_MS = 0.001  # 每 1 毫秒发送一次
connection_counter = 0
counter_lock = threading.Lock()  # 保证多线程安全递增连接ID

def recv_thread(conn, conn_id):
    while True:
        try:
            data = conn.recv(1024)
            if not data:
                print(f"[-] Connection ID {conn_id} closed by client")
                break
            print(f"[Received from conn_id={conn_id}]: {data.decode(errors='ignore').strip()}")
        except ConnectionResetError:
            print(f"[-] Connection ID {conn_id} disconnected")
            break
        except Exception:
            pass  # 可加日志处理

def send_thread(conn, conn_id):
    counter = 0
    while True:
        try:
            message = f"conn_id={conn_id} msg: {counter}\n"
            conn.sendall(message.encode('utf-8'))
            counter += 1
            time.sleep(INTERVAL_MS)
        except (ConnectionResetError, BrokenPipeError):
            print(f"[-] Connection ID {conn_id} send thread closed")
            break

def handle_client(conn, addr, conn_id):
    print(f"[+] Connected {addr}, conn_id={conn_id}")

    t_recv = threading.Thread(target=recv_thread, args=(conn, conn_id), daemon=True)
    t_send = threading.Thread(target=send_thread, args=(conn, conn_id), daemon=True)
    t_recv.start()
    t_send.start()

    t_recv.join()
    t_send.join()
    conn.close()
    print(f"[-] Connection {conn_id} fully closed")


def start_server(port):
    host = '0.0.0.0'
    global connection_counter

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.bind((host, port))
        server_socket.listen()
        print(f"[+] TCP Server listening on {host}:{port}")

        while True:
            conn, addr = server_socket.accept()
            with counter_lock:
                connection_counter += 1
                conn_id = connection_counter
            thread = threading.Thread(target=handle_client, args=(conn, addr, conn_id), daemon=True)
            thread.start()

def parse_args():
    parser = argparse.ArgumentParser(description="TCP Server that sends message every 1ms.")
    parser.add_argument('--port', type=int, default=2345, help="Port to listen on (default: 2345)")
    return parser.parse_args()

if __name__ == "__main__":
    args = parse_args()
    start_server(args.port)
