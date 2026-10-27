#include "SensorData.h"

SensorData::SensorData()
	: timestamp(QDateTime())
	, deviceId(0)
	, sensorId(0)
	, dataType(0)
	, value(0){
}
SensorData::SensorData(QDateTime timestamp, quint8 deviceId, quint8 sensorId, quint8 dataType, double value)
	: timestamp(timestamp)
	, deviceId(deviceId)
	, sensorId(sensorId)
	, dataType(dataType)
	, value(value){
}


QString SensorData::typeName() const {
	if (dataType == DATA_TYPE_TEMP_INT16){
		return QString("温度");
	}else {
		return QString("未知数据类型");
	}
}