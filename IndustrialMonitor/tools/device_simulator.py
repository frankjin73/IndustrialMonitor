"""
工业设备模拟器
通过虚拟串口 COM11 发送模拟传感器数据

帧结构:
    帧头   : 0xAA 0x55            (2 字节, 大端)
    长度   : 数据区字节数          (1 字节, 0~255)
    命令字 : 1 字节               (0x01 = 传感器数据上报)
    数据区 :
        时间戳    : 4 字节  uint32 大端 (Unix 秒)
        设备 ID   : 1 字节  uint8
        传感器 ID : 1 字节  uint8
        数据类型  : 1 字节  uint8 (0x01 = 温度 int16×100)
        数值      : 2 字节  int16 大端
    校验   : CRC16-MODBUS         (2 字节, 小端)

CRC 计算范围: 帧头 + 长度 + 命令字 + 数据区 (不含 CRC 本身)
"""
import serial
import time
import random
import sys
import struct

# ---------------- 协议常量 ----------------
HEADER = b"\xAA\x55"
MAX_DATA_LEN = 255

CMD_SENSOR_REPORT = 0x01          # 命令字: 传感器数据上报
DATA_TYPE_TEMP_INT16 = 0x01       # 数据类型: 温度 int16 (×100)

DEVICE_ID_DEFAULT = 0x01
SENSOR_ID_DEFAULT = 0x01

PORT = 'COM11'
BAUD_RATE = 115200

TEMP_MIN = 25.0
TEMP_MAX = 30.0


# ---------------- CRC16-MODBUS ----------------
def crc16_modbus(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc & 0xFFFF


# ---------------- 组帧 ----------------
def build_frame(cmd: int, data: bytes = b"") -> bytes:
    if not 0 <= cmd <= 0xFF:
        raise ValueError("cmd 必须是 0~255")
    if len(data) > MAX_DATA_LEN:
        raise ValueError(f"数据区过长: {len(data)} > {MAX_DATA_LEN}")
    body = HEADER + bytes([len(data), cmd]) + data
    crc = crc16_modbus(body)
    return body + struct.pack("<H", crc)     # CRC 小端


def build_sensor_payload(timestamp: int, device_id: int, sensor_id: int,
                         data_type: int, value_bytes: bytes) -> bytes:
    """数据区: ts(4B BE) + devID + senID + type + value"""
    return (struct.pack(">I", timestamp & 0xFFFFFFFF)
            + bytes([device_id & 0xFF, sensor_id & 0xFF, data_type & 0xFF])
            + value_bytes)


def build_temp_frame(temperature: float,
                     device_id: int = DEVICE_ID_DEFAULT,
                     sensor_id: int = SENSOR_ID_DEFAULT,
                     timestamp: int = None) -> bytes:
    """温度帧: 数值为 int16 大端 (温度 × 100)"""
    if timestamp is None:
        timestamp = int(time.time())
    raw = int(round(temperature * 100))
    raw = max(-32768, min(32767, raw))       # 防溢出
    payload = build_sensor_payload(
        timestamp, device_id, sensor_id,
        DATA_TYPE_TEMP_INT16,
        struct.pack(">h", raw)
    )
    return build_frame(CMD_SENSOR_REPORT, payload)


# ---------------- 主程序 ----------------
def main():
    assert crc16_modbus(b"123456789") == 0x4B37, "CRC 实现有误"

    try:
        ser = serial.Serial(
            port=PORT, baudrate=BAUD_RATE,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=1, write_timeout=1,
        )
        print(f"[模拟器] 已打开 {PORT} @ {BAUD_RATE}")
        print("[模拟器] 按 Ctrl+C 停止")
    except serial.SerialException as e:
        print(f"[错误] 无法打开 {PORT}: {e}")
        print("请确认 com0com 已安装，且 COM11 未被占用")
        sys.exit(1)

    try:
        while True:
            temperature = round(random.uniform(TEMP_MIN, TEMP_MAX), 2)
            ts = int(time.time())
            frame = build_temp_frame(temperature, timestamp=ts)

            try:
                ser.write(frame)
                ser.flush()
            except serial.SerialTimeoutException:
                print("[警告] 写入超时，对端可能未读取数据")
                continue
            except serial.SerialException as e:
                print(f"[错误] 串口写入失败: {e}")
                break

            print(f"[发送] {time.strftime('%H:%M:%S', time.localtime(ts))} "
                  f"T={temperature:.2f}℃  frame={frame.hex(' ').upper()}")
            time.sleep(0.01)

    except KeyboardInterrupt:
        print("\n[模拟器] 已停止")
    finally:
        if ser.is_open:
            ser.close()
            print("[模拟器] 串口已关闭")


if __name__ == '__main__':
    main()