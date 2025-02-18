import socket
import signal
import sys
import time

TO_STOP = False

# 定义信号处理函数，用于处理 Ctrl + C 信号
def signal_handler(sig, frame):
    print('You pressed Ctrl+C! Closing the server...')
    global TO_STOP
    TO_STOP = True

# 注册信号处理函数，处理 SIGINT 信号（Ctrl + C）
signal.signal(signal.SIGINT, signal_handler)

# 定义服务器的主机和端口
HOST = '127.0.0.1'  # 本地主机
PORT = 8888  # 监听的端口

# 创建一个 TCP 套接字
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# 设置套接字选项，允许地址重用
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
# 设置超时时间
server_socket.settimeout(1.0)
# 绑定主机和端口
server_socket.bind((HOST, PORT))
# 开始监听，最大连接数为 5
server_socket.listen(5)
print(f"Server is listening on {HOST}:{PORT}")

while True:
    if TO_STOP:
        break;
    client_socket = None
    try:
        # 接受客户端连接
        client_socket, client_address = server_socket.accept()
        print(f"Accepted connection from {client_address}")

        # 接收客户端发送的数据
        data = client_socket.recv(1024)
        if data:
            message = data.decode('utf-8')
            print(f"Received: {message}")

            # 回复客户端
            response = f"Server received: {message}"
            client_socket.sendall(response.encode('utf-8'))
    except socket.timeout as e:
        pass
    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        if client_socket:
            client_socket.close()

server_socket.close()