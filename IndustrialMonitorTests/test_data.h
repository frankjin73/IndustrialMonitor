#include <QByteArray>
#include "DataParser.h"

/*
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
*/

QByteArray makeFrame(quint8 sensorId, double value) {
	QByteArray frame;

    QByteArray header; 
    header = QByteArray::fromHex("AA55");
    //header.append(char(0xAA)); header.append(char(0x55));

    QByteArray len; len.append(char(0x09));
    QByteArray cmd; cmd.append(char(0x01));
    QByteArray timestamp; 
    {
        timestamp.append(char(0x6A)); timestamp.append(char(0xFF)); timestamp.append(char(0x8D)); timestamp.append(char(0x80));
    }
    QByteArray deviceId; deviceId.append(char(0x01));
    QByteArray sensorIdA; sensorIdA.append(char(sensorId));
    QByteArray dataType; dataType.append(char(0x01));
    QByteArray valueA;
    {
        auto v = static_cast<quint16>(value*100);
        valueA.append(char(static_cast<quint8>((v >> 8) & 0xFF)));  // 高字节
        valueA.append(char(static_cast<quint8>(v & 0xFF)));         // 低字节
    }

    QByteArray body = timestamp + deviceId + sensorIdA + dataType + valueA;

    QByteArray crcA;
    {
        quint16 crc = DataParser::crc16Modbus(header + len + cmd + body);
        crcA.append(char(static_cast<quint8>(crc & 0xFF)));         // 低字节
        crcA.append(char(static_cast<quint8>((crc >> 8) & 0xFF)));  // 高字节
    }

    frame = header + len + cmd + body + crcA;

    return frame;
}

