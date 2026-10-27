#pragma once

#include <QList>
#include <QByteArray>
#include "SensorData.h"

class DataParser
{
public:

	QList<SensorData> parse(const QByteArray& chunk);

	//纯计算函数，不碰任何成员变量，所以声明为 static
	static quint16 crc16Modbus(const QByteArray& crcData);

	QByteArray m_buffer;	// 接收缓冲区
	static constexpr int maxFrameLen = 300;   // 帧长上限，防恶意长度
};

