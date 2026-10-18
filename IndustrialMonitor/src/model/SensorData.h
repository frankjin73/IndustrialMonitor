#pragma once
#include <qtypes.h>
#include <qstring.h>

#define DATA_TYPE_TEMP_INT16  0x01

class SensorData
{
	/*
	时间戳    : 4 字节  uint32 大端 (Unix 秒)
        设备 ID   : 1 字节  uint8
        传感器 ID : 1 字节  uint8
        数据类型  : 1 字节  uint8 (0x01 = 温度 int16×100)
        数值      : 2 字节  int16 大端
	*/
public:
    SensorData();
    SensorData(qint32 timestamp, qint8 deviceId,qint8 sensorId,qint8 dataType,qint16 value);
    QString typeName() const;

    qint32 timestamp;
    qint8 deviceId;
    qint8 sensorId;
    qint8 dataType;
    qint16 value;
};

