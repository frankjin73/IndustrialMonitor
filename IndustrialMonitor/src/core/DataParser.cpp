#include "DataParser.h"

QList<SensorData> DataParser::parse(const QByteArray& chunk) {
	const QByteArray header = QByteArray::fromHex("AA55");
	const int headerLen = header.size();
	const int minFrameLen = headerLen + 1 + 1 + 0 + 2;

	QList<SensorData> result;

	while (true) {
		//找帧头
		int pos = m_buffer.indexOf(header);
		if (pos < 0) {
			if (m_buffer.size() > 1) {
				//当前帧没找到帧头，则帧的最后一字节可能是0xAA，把最后一字节留到下个帧处理
				m_buffer = m_buffer.right(1);
			}
			break;
		}

		if (pos > 0) {
			m_buffer.remove(0, pos);
		}

		//是否能读到长度字段
		if (m_buffer.size() < headerLen + 1) { break; }

		quint8 payLoadLen = static_cast<quint8>(m_buffer.at(headerLen));
		int frameLen = headerLen + 1 + 1 + payLoadLen + 2;

		//超过最大帧长
		if (frameLen > maxFrameLen) {
			m_buffer.remove(0, 1);
			continue;
		}

		//收到半帧，留到下一帧一起处理
		if (m_buffer.size() < frameLen) {
			break;
		}
			
		//取帧
		QByteArray frame = m_buffer.left(frameLen);

		//CRC16-MODBUS(2 字节, 小端)
		quint16 recCrc = static_cast<quint8>(frame.at(frameLen - 2)) | (static_cast<quint8>(frame.at(frameLen - 1)) << 8);
		
		//CRC 计算范围 : 帧头 + 长度 + 命令字 + 数据区(不含 CRC 本身)
		QByteArray crcData = frame.left(frameLen - 2);
		if (recCrc != crc16Modbus(crcData)) {
			m_buffer.remove(0, 1);
			continue;
		}

		quint8 cmd = static_cast<quint8>(frame.at(headerLen + 1));
		QByteArray payload = frame.mid(headerLen + 2, payLoadLen);

		//解析成 SensorData
		if (cmd == CMD_SENSOR_REPORT) {
			SensorData d;
			d.timestamp = QDateTime::currentDateTime();
			d.deviceId = static_cast<quint8>(payload.at(4));
			d.sensorId = static_cast<quint8>(payload.at(5));
			d.dataType = static_cast<quint8>(payload.at(6));
			d.value = static_cast<double>((payload.at(7) << 8) | quint8(payload.at(8)))/100;
			result.append(d);
		}
		m_buffer.remove(0, frameLen);

	}
	return result;
}

quint16 DataParser::crc16Modbus(const QByteArray& crcData){
	quint16 crc = 0xFFFF;

	for (auto byte : crcData) {
		crc ^= static_cast<quint8>(byte);
		for (int i = 0; i < 8; ++i) {
			if (crc & 0x0001) {
				crc = (crc >> 1) ^ 0xA001;      // 右移，异或多项式（0x8005 反转）
			}
			else {
				crc >>= 1;
			}
		}
	}

	return crc;
}