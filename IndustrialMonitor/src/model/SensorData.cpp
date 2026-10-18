#include "SensorData.h"

SensorData::SensorData()
	: timestamp(0)
	, deviceId(0)
	, sensorId(0)
	, dataType(0)
	, value(0){
}
SensorData::SensorData(qint32 timestamp, qint8 deviceId, qint8 sensorId, qint8 dataType, qint16 value)
	: timestamp(timestamp)
	, deviceId(deviceId)
	, sensorId(sensorId)
	, dataType(dataType)
	, value(value){
}


QString SensorData::typeName() const {
	if (value == DATA_TYPE_TEMP_INT16){
		return QString("温度");
	}else {
		return QString("未知数据类型");
	}
}