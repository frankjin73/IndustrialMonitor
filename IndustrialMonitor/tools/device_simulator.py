"""
工业设备模拟器
通过虚拟串口 COM11 发送模拟传感器数据
"""
import serial
import time
import random
import sys

PORT = 'COM11'
BAUD_RATE = 115200

# 温度范围（摄氏度）
TEMP_MIN = 25.0
TEMP_MAX = 30.0


def main():
    ser = None
    try:
        ser = serial.Serial(
            port=PORT,
            baudrate=BAUD_RATE,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=1,
            write_timeout=1,   # 写操作超时，防止卡死
        )
        print(f"[模拟器] 已打开 {PORT} @ {BAUD_RATE}")
        print("[模拟器] 按 Ctrl+C 停止")
    except serial.SerialException as e:
        print(f"[错误] 无法打开 {PORT}: {e}")
        print("请确认 com0com 已安装，且 COM11 未被占用")
        sys.exit(1)

    try:
        while True:
            # 模拟温度值 25.0 ~ 30.0
            temperature = random.uniform(TEMP_MIN, TEMP_MAX)
            line = f"T:{temperature:.2f}\n"
            try:
                ser.write(line.encode('ascii'))
                ser.flush()   # 确保数据真正发出
            except serial.SerialTimeoutException:
                print("[警告] 写入超时，对端可能未读取数据")
                continue
            except serial.SerialException as e:
                print(f"[错误] 串口写入失败: {e}")
                break

            print(f"[发送] {line.strip()}")
            time.sleep(1)

    except KeyboardInterrupt:
        print("\n[模拟器] 已停止")
    finally:
        if ser is not None and ser.is_open:
            ser.close()
            print("[模拟器] 串口已关闭")


if __name__ == '__main__':
    main()