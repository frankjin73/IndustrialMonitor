#pragma once
#include <qtypes.h>
#include <qstring.h>
#include <QDateTime>

#define DATA_TYPE_TEMP_INT16  0x01  //温度
#define CMD_SENSOR_REPORT  0x01

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
    SensorData(QDateTime timestamp, quint8 deviceId,quint8 sensorId,quint8 dataType, double value);
    QString typeName() const;

    QDateTime timestamp;
    quint8 deviceId;
    quint8 sensorId;
    quint8 dataType;
    double value;
};

