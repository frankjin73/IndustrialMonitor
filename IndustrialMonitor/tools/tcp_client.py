import socket
import json
import sys
import time

def start_client(host='127.0.0.1', port=8888):
    # 1. 创建 TCP Socket
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    try:
        # 2. 连接服务端
        print(f"[*] 正在连接到 {host}:{port} ...")
        client.connect((host, port))
        print("[+] 连接成功！等待接收广播数据...\n")
    except ConnectionRefusedError:
        print("[-] 连接失败，请确认服务端已启动且端口正确。")
        return
    except Exception as e:
        print(f"[-] 发生错误: {e}")
        return

    # 缓冲区，用于处理 TCP 粘包和半包
    buffer = b""
    
    try:
        while True:
            # 3. 接收数据
            data = client.recv(4096)
            if not data:
                print("[-] 服务端断开了连接。")
                break
            
            # 4. 将新数据追加到缓冲区
            buffer += data
            
            # 5. 按换行符 '\n' 切割数据（假设服务端每条 JSON 后都加了 '\n'）
            while b'\n' in buffer:
                # 取出完整的一行
                line, buffer = buffer.split(b'\n', 1)
                
                # 去除两端的空白字符
                line = line.strip()
                if not line:
                    continue
                
                # 6. 解析 JSON
                try:
                    json_str = line.decode('utf-8')
                    parsed_data = json.loads(json_str)
                    
                    # 7. 逐行打印输出
                    # 使用 ensure_ascii=False 保证中文正常显示
                    print(f"[接收] {json.dumps(parsed_data, ensure_ascii=False)}")
                    
                except UnicodeDecodeError:
                    print(f"[警告] 收到非 UTF-8 数据: {line}")
                except json.JSONDecodeError:
                    print(f"[警告] 收到非 JSON 格式数据: {line.decode('utf-8', errors='ignore')}")

    except KeyboardInterrupt:
        print("\n[*] 手动中断，退出客户端。")
    except Exception as e:
        print(f"[-] 接收数据异常: {e}")
    finally:
        client.close()

if __name__ == "__main__":
    # 支持命令行传参：python tcp_client.py <ip> <port>
    target_host = sys.argv[1] if len(sys.argv) > 1 else '127.0.0.1'
    target_port = int(sys.argv[2]) if len(sys.argv) > 2 else 8888
    
    start_client(target_host, target_port)